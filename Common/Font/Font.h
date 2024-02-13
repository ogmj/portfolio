#pragma once

struct Font
{
	Font(const char* szFace, int nWidth = 32, int nHeight = 16)
	{
		m_hFont = CreateCustomFont(szFace, nWidth, nHeight);
	}

	~Font()
	{
		DeleteObject(m_hFont);
	}

	operator HFONT() const
	{
		return m_hFont;
	}

	static HFONT CreateCustomFont(const char* szFace, int nWidth = 32, int nHeight = 16)
	{
		return CreateFont(
			nHeight,				// int nHeight,               // height of font
			nWidth,					// int nWidth,                // average character width
			0,						// int nEscapement,           // angle of escapement
			0,						// int nOrientation,          // base-line orientation angle
			FW_NORMAL,				// int fnWeight,              // font weight
			0,						// DWORD fdwItalic,           // italic attribute option
			0,						// DWORD fdwUnderline,        // underline attribute option
			0,						// DWORD fdwStrikeOut,        // strikeout attribute option
			DEFAULT_CHARSET,		// DWORD fdwCharSet,          // character set identifier
			OUT_DEFAULT_PRECIS,		// DWORD fdwOutputPrecision,  // output precision
			CLIP_DEFAULT_PRECIS,	// DWORD fdwClipPrecision,    // clipping precision
			ANTIALIASED_QUALITY,	// DWORD fdwQuality,          // output quality (DEFAULT_QUALITY)
			FF_DONTCARE,			// DWORD fdwPitchAndFamily,   // pitch and family
			szFace);
	}

private:

	HFONT m_hFont;
};
