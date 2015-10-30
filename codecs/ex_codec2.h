/*! \file
 * \brief 8-bit raw data
 *
 * Copyright (C) 2012, 2012 Ed W and David Rowe
 *
 * Distributed under the terms of the GNU General Public License
 *
 */

static uint8_t ex_codec2[] = {
    0xea,0xca,0x14,0x85,0x91,0x78
};

static struct ast_frame *codec2_sample(void)
{
	static struct ast_frame f = {
		.frametype = AST_FRAME_VOICE,
		.datalen = sizeof(ex_codec2),
		.samples = CODEC2_SAMPLES,
		.mallocd = 0,
		.offset = 0,
		.src = __PRETTY_FUNCTION__,
		.data.ptr = ex_codec2,
	};
	
	f.subclass.format = ast_format_codec2;

	return &f;
}
