#include "PreRTS.h"
#include "GameClient/Mouse.h"

#include <cstdio>

namespace {
class HeadlessMouse final : public Mouse
{
public:
	void initCursorResources() override { ++resources; }
	void setCursor(MouseCursor cursor) override { m_currentCursor = cursor; }
	void capture() override { captured = TRUE; }
	void releaseCapture() override { captured = FALSE; }
	Bool tooltip_empty() const { return m_isTooltipEmpty; }
	int resources = 0;
	Bool captured = FALSE;
protected:
	UnsignedByte getMouseEvent(MouseIO *, Bool) override { return 0; }
};
int failures;
void check(bool value, const char *message) { if (!value) { std::fprintf(stderr, "FAIL: %s\n", message); ++failures; } }
}

int main()
{
	for (int generation = 0; generation != 2; ++generation)
	{
		check(TheMouse == NULL, "stale mouse provider survived generation");
		HeadlessMouse mouse;
		TheMouse = &mouse;
		mouse.initCursorResources();
		mouse.setCursorTooltip(UnicodeString::TheEmptyString);
		check(mouse.tooltip_empty() && mouse.getCursorTooltipDelay() == -1 && mouse.resources == 1,
			"source empty cursor-tooltip clear/default changed");
		mouse.capture(); mouse.releaseCapture();
		check(!mouse.captured, "headless mouse capture release changed");
		mouse.reset();
		check(mouse.tooltip_empty(), "source mouse reset retained a tooltip");
		TheMouse = NULL;
	}
	check(TheMouse == NULL, "mouse provider removal retained ownership");
	std::puts("original generated mouse provider: generations=2 resources=0");
	return failures ? 1 : 0;
}
