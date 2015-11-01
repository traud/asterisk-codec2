/*! \file
 *
 * \brief Translate between signed linear and Codec 2
 *
 * \ingroup codecs
 */

/*** MODULEINFO
	<depend>codec2</depend>
 ***/

#include "asterisk.h"

#include "asterisk/codec.h"             /* for AST_MEDIA_TYPE_AUDIO */
#include "asterisk/frame.h"             /* for ast_frame            */
#include "asterisk/linkedlists.h"       /* for AST_LIST_NEXT, etc   */
#include "asterisk/logger.h"            /* for ast_log, etc         */
#include "asterisk/module.h"
#include "asterisk/translate.h"         /* for ast_trans_pvt, etc   */

#include <codec2/codec2.h>

#define BUFFER_SAMPLES    8000
#define CODEC2_SAMPLES    160
#define	CODEC2_FRAME_LEN  6

/* Sample frame data */
#include "asterisk/slin.h"
#include "ex_codec2.h"

struct codec2_translator_pvt {
	struct CODEC2 *codec2; /* May be encoder or decoder */
	int16_t buf[BUFFER_SAMPLES];
};

static int codec2_new(struct ast_trans_pvt *pvt)
{
	struct codec2_translator_pvt *tmp = pvt->pvt;

	tmp->codec2 = codec2_create(CODEC2_MODE_2400);

	if (!tmp->codec2) {
		ast_log(LOG_ERROR, "Error creating Codec 2 conversion\n");
		return -1;
	}

	return 0;
}

/*! \brief decode and store in outbuf. */
static int codec2tolin_framein(struct ast_trans_pvt *pvt, struct ast_frame *f)
{
	struct codec2_translator_pvt *tmp = pvt->pvt;
	int x;

	for (x = 0; x < f->datalen; x += CODEC2_FRAME_LEN) {
		unsigned char *src = f->data.ptr + x;
		int16_t *dst = pvt->outbuf.i16 + pvt->samples;

		codec2_decode(tmp->codec2, dst, src);

		pvt->samples += CODEC2_SAMPLES;
		pvt->datalen += CODEC2_SAMPLES * 2;
	}

	return 0;
}

/*! \brief store samples into working buffer for later decode */
static int lintocodec2_framein(struct ast_trans_pvt *pvt, struct ast_frame *f)
{
	struct codec2_translator_pvt *tmp = pvt->pvt;

	if (pvt->samples + f->samples > BUFFER_SAMPLES) {
		ast_log(LOG_WARNING, "Out of buffer space\n");
		return -1;
	}
	memcpy(tmp->buf + pvt->samples, f->data.ptr, f->datalen);
	pvt->samples += f->samples;

	return 0;
}

/*! \brief encode and produce a frame */
static struct ast_frame *lintocodec2_frameout(struct ast_trans_pvt *pvt)
{
	struct codec2_translator_pvt *tmp = pvt->pvt;
	struct ast_frame *result = NULL;
	struct ast_frame *last = NULL;
	int samples = 0; /* output samples */

	while (pvt->samples >= CODEC2_SAMPLES) {
		struct ast_frame *current;

		/* Encode a frame of data */
		codec2_encode(tmp->codec2, pvt->outbuf.uc, tmp->buf + samples);

		samples += CODEC2_SAMPLES;
		pvt->samples -= CODEC2_SAMPLES;

		current = ast_trans_frameout(pvt, CODEC2_FRAME_LEN, CODEC2_SAMPLES);

		if (!current) {
			continue;
		} else if (last) {
			AST_LIST_NEXT(last, frame_list) = current;
		} else {
			result = current;
		}
		last = current;
	}

	/* Move the data at the end of the buffer to the front */
	if (samples) {
		memmove(tmp->buf, tmp->buf + samples, pvt->samples * 2);
	}

	return result;
}

static void codec2_destroy_stuff(struct ast_trans_pvt *pvt)
{
	struct codec2_translator_pvt *tmp = pvt->pvt;

	if (tmp->codec2) {
		codec2_destroy(tmp->codec2);
	}
}

static struct ast_translator codec2tolin = {
	.name = "codec2tolin",
	.src_codec = {
		.name = "codec2",
		.type = AST_MEDIA_TYPE_AUDIO,
		.sample_rate = 8000,
	},
	.dst_codec = {
		.name = "slin",
		.type = AST_MEDIA_TYPE_AUDIO,
		.sample_rate = 8000,
	},
	.format = "slin8",
	.newpvt = codec2_new,
	.framein = codec2tolin_framein,
	.destroy = codec2_destroy_stuff,
	.sample = codec2_sample,
	.desc_size = sizeof(struct codec2_translator_pvt),
	.buffer_samples = BUFFER_SAMPLES,
	.buf_size = BUFFER_SAMPLES * 2,
};

static struct ast_translator lintocodec2 = {
	.name = "lintocodec2",
	.src_codec = {
		.name = "slin",
		.type = AST_MEDIA_TYPE_AUDIO,
		.sample_rate = 8000,
	},
	.dst_codec = {
		.name = "codec2",
		.type = AST_MEDIA_TYPE_AUDIO,
		.sample_rate = 8000,
	},
	.format = "codec2",
	.newpvt = codec2_new,
	.framein = lintocodec2_framein,
	.frameout = lintocodec2_frameout,
	.destroy = codec2_destroy_stuff,
	.sample = slin8_sample,
	.desc_size = sizeof(struct codec2_translator_pvt),
	.buf_size = (BUFFER_SAMPLES * CODEC2_FRAME_LEN + CODEC2_SAMPLES - 1) / CODEC2_SAMPLES,
};

static int unload_module(void)
{
	int res;

	res = ast_unregister_translator(&lintocodec2);
	res |= ast_unregister_translator(&codec2tolin);

	return res;
}

static int load_module(void)
{
	int res;

	res = ast_register_translator(&codec2tolin);
	res |= ast_register_translator(&lintocodec2);

	if (res) {
		unload_module();
		return AST_MODULE_LOAD_FAILURE;
	}

	return AST_MODULE_LOAD_SUCCESS;
}

AST_MODULE_INFO_STANDARD(ASTERISK_GPL_KEY, "Codec 2 Coder/Decoder");
