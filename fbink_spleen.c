static const uint16_t*
    spleen_get_bitmap(uint32_t codepoint)
{
	if (codepoint >= 0x20 && codepoint <= 0x7f) {
		return spleen_block1[codepoint - 0x20];
	} else if (codepoint >= 0xa0 && codepoint <= 0xff) {
		return spleen_block2[codepoint - 0xa0];
	} else {
		WARN("Codepoint U+%04X is not covered by this font", codepoint);
		return spleen_block1[0];
	}
}
