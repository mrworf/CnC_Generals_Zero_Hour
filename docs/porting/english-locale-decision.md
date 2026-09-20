# M8 English text-layout decision

The selected corpus manifest identifies the locale as `English` and records `Arial Unicode MS` as a system-family choice with `LocalFontFile` disabled. English is left-to-right and the selected menu, tooltip, subtitle/caption, name, and chat cases need neither contextual glyph substitution nor bidirectional paragraph ordering.

M8 therefore keeps FreeType plus Fontconfig as the complete selected-locale stack. HarfBuzz and FriBidi are not dependencies for this corpus. `ui_font_tests` exercises ASCII and non-ASCII text, metrics, wrapping, truncation, deliberate missing-glyph replacement, memory-face byte lifetime, and Fontconfig fallback. This decision does not promise equivalent layout for other historical retail locales; selecting one of those locales requires rerunning the gate in the port plan.
