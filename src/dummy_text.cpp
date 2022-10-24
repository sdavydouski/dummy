internal u32
GetCharacterGlyphIndex(font *Font, wchar Character)
{
    u32 GlyphIndex = 0;

    for (u32 CodepointsRangeIndex = 0; CodepointsRangeIndex < Font->CodepointsRangeCount; ++CodepointsRangeIndex)
    {
        codepoints_range *CodepointsRange = Font->CodepointsRanges + CodepointsRangeIndex;

        u32 Index = Character - CodepointsRange->Start;

        if (Index < CodepointsRange->Count)
        {
            GlyphIndex += Index;
            break;
        }
        else
        {
            GlyphIndex += CodepointsRange->Count;
        }
    }

    return GlyphIndex;
}

inline f32
GetHorizontalAdvanceForPair(font *Font, wchar Character, wchar NextCharacter)
{
    f32 Result = 0.f;

    i32 RowIndex = GetCharacterGlyphIndex(Font, Character);
    i32 ColumnIndex = GetCharacterGlyphIndex(Font, NextCharacter);

    if (RowIndex >= 0 && ColumnIndex >= 0)
    {
        Assert((RowIndex * Font->GlyphCount + ColumnIndex) < Font->HorizontalAdvanceTableCount);

        Result = *(Font->HorizontalAdvanceTable + RowIndex * Font->GlyphCount + ColumnIndex);
    }

    return Result;
}

inline glyph *
GetCharacterGlyph(font *Font, wchar Character)
{
    u32 GlyphIndex = GetCharacterGlyphIndex(Font, Character);
    glyph *Result = Font->Glyphs + GlyphIndex;

    return Result;
}

inline vec2
GetTextBounds(wchar *Text, font *Font)
{
    vec2 Result = vec2(0.f);

    for (wchar *At = Text; *At; ++At)
    {
        wchar Character = *At;
        wchar NextCharacter = *(At + 1);

        glyph *GlyphInfo = GetCharacterGlyph(Font, Character);
        f32 HorizontalAdvance = GetHorizontalAdvanceForPair(Font, Character, NextCharacter);

        Result.x += HorizontalAdvance;
        Result.y = Max(Result.y, GlyphInfo->CharacterSize.y + GlyphInfo->Alignment.y);
    }

    return Result;
}

inline vec2
GetTextStartPosition(wchar *Text, font *Font, draw_text_alignment Alignment, vec2 Position, f32 TextScale, f32 UnitsPerPixel)
{
    vec2 Result = vec2(0.f);

    vec2 TextBounds = GetTextBounds(Text, Font) * TextScale * UnitsPerPixel;

    switch (Alignment)
    {
        case DrawText_AlignLeft:
        {
            Result = vec2(Position.x, Position.y);
            break;
        }
        case DrawText_AlignCenter:
        {
            Result = vec2(Position.x - TextBounds.x / 2.f, Position.y);
            break;
        }
        case DrawText_AlignRight:
        {
            Result = vec2(Position.x - TextBounds.x, Position.y);
            break;
        }
        default:
        {
            Assert(!"Invalid text alignment");
            break;
        }
    }

    return Result;
}
