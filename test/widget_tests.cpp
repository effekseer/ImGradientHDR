#include <ImGradientHDR.h>
#include <imgui.h>

#include <cmath>
#include <cstdio>

#define CHECK(condition) do { if (!(condition)) { std::fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); return false; } } while (false)

struct FrameResult
{
	bool Changed;
	bool Edited;
	bool DeactivatedAfterEdit;
	ImGuiID ActiveId;
};

class Widget
{
public:
	ImGradientHDRState State{};
	ImGradientHDRTemporaryState Temporary;
	ImVec2 Origin;
	float Width = 0.0f;

	Widget()
	{
		ImGui::CreateContext();
		auto& io = ImGui::GetIO();
		io.IniFilename = nullptr;
		io.DisplaySize = {800, 600};
		io.DeltaTime = 1.0f / 60.0f;
		io.ConfigInputTrickleEventQueue = false;
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
		State.AddColorMarker(0.2f, {1, 0, 0}, 1);
		State.AddColorMarker(0.8f, {0, 0, 1}, 1);
		State.AddAlphaMarker(0.2f, 0.25f);
		State.AddAlphaMarker(0.8f, 0.75f);
		Frame({-100, -100}, false);
		Frame({-100, -100}, false);
	}

	~Widget() { ImGui::DestroyContext(); }

	float MarkerY(bool color) const
	{
		return Origin.y + 7.0f + (color ? 35.0f + ImGui::GetStyle().ItemSpacing.y * 2.0f : 0.0f);
	}

	FrameResult Frame(ImVec2 mouse, bool down, bool disabled = false, float availableWidth = -1.0f)
	{
		auto& io = ImGui::GetIO();
		io.AddMousePosEvent(mouse.x, mouse.y);
		io.AddMouseButtonEvent(0, down);
		ImGui::NewFrame();
		ImGui::SetNextWindowPos({0, 0});
		ImGui::SetNextWindowSize({400, 300});
		ImGui::Begin("Gradient", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		if (availableWidth >= 0.0f)
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - availableWidth);
		Origin = ImGui::GetCursorScreenPos();
		Width = ImGui::GetContentRegionAvail().x - 10.0f;
		ImGui::BeginDisabled(disabled);
		const bool changed = ImGradientHDR(10, State, Temporary);
		FrameResult result{changed, ImGui::IsItemEdited(), ImGui::IsItemDeactivatedAfterEdit(),
			ImGui::IsItemActive() ? ImGui::GetItemID() : 0};
		ImGui::EndDisabled();
		ImGui::End();
		ImGui::Render();
		return result;
	}
};

bool Dragging(bool color)
{
	Widget widget;
	const float x = widget.Origin.x + static_cast<int>(widget.Width * 0.2f);
	const float y = widget.MarkerY(color);
	widget.Frame({x, y}, false);
	const auto pressed = widget.Frame({x, y}, true);
	CHECK(pressed.ActiveId != 0);
	const auto moved = widget.Frame({x + widget.Width * 0.7f, y}, true);
	CHECK(moved.Changed && moved.Edited);
	CHECK(widget.Temporary.draggingIndex == 1);
	const auto crossing = widget.Frame({x + widget.Width * 0.72f, y}, true);
	CHECK(crossing.Changed && crossing.Edited && crossing.ActiveId == pressed.ActiveId);
	CHECK(widget.Temporary.selectedIndex == 1);
	const auto released = widget.Frame({x + widget.Width * 0.72f, y}, false);
	CHECK(released.DeactivatedAfterEdit && !released.Changed);
	CHECK(widget.Temporary.draggingIndex == -1);
	CHECK(color ? widget.State.Colors[1].Color[0] == 1.0f : widget.State.Alphas[1].Alpha == 0.25f);
	return true;
}

bool DisabledAndClamping(bool color)
{
	Widget widget;
	const float x = widget.Origin.x + static_cast<int>(widget.Width * 0.2f);
	const float y = widget.MarkerY(color);
	widget.Frame({x, y}, false);
	widget.Frame({x, y}, true);
	CHECK(widget.Frame({x + 20, y}, true).Changed);
	const float before = color ? widget.State.Colors[0].Position : widget.State.Alphas[0].Position;
	CHECK(!widget.Frame({x + 40, y}, true, true).Changed);
	CHECK((color ? widget.State.Colors[0].Position : widget.State.Alphas[0].Position) == before);
	CHECK(widget.Temporary.draggingIndex == -1);
	CHECK(!widget.Frame({x + 60, y}, true).Changed);
	widget.Frame({x + 60, y}, false);
	const float keyX = widget.Origin.x + static_cast<int>(widget.Width * before);
	widget.Frame({keyX, y}, false);
	widget.Frame({keyX, y}, true);
	CHECK(widget.Frame({-100, y}, true).Changed);
	const auto clamped = widget.Frame({-120, y}, true);
	CHECK(!clamped.Changed && !clamped.Edited);
	CHECK(widget.Frame({-120, y}, false).DeactivatedAfterEdit);
	return true;
}

bool Insertion(bool color)
{
	Widget widget;
	widget.State.AddColorMarker(0.5f, {0, 1, 0}, 1);
	widget.State.AddAlphaMarker(0.5f, 0.5f);
	widget.Temporary.selectedMarkerType = color ? ImGradientHDRMarkerType::Color : ImGradientHDRMarkerType::Alpha;
	widget.Temporary.selectedIndex = 2;
	const ImVec2 mouse{widget.Origin.x + widget.Width * 0.35f, widget.MarkerY(color)};
	widget.Frame(mouse, false);
	const auto added = widget.Frame(mouse, true);
	CHECK(added.Changed && added.Edited);
	CHECK(widget.Temporary.selectedIndex == 3);
	CHECK((color ? widget.State.Colors[3].Position : widget.State.Alphas[3].Position) == 0.8f);
	CHECK(widget.Frame(mouse, false).DeactivatedAfterEdit);
	return true;
}

bool WidthAndHdr()
{
	Widget widget;
	for (float width : {0.0f, 5.0f, 10.0f, 10.5f, 20.0f})
		CHECK(!widget.Frame({-100, -100}, false, false, width).Changed);
	widget.State = ImGradientHDRState{};
	widget.State.AddColorMarker(0, {1, 0, 0}, 0);
	widget.State.AddColorMarker(1, {1, 0, 0}, 2);
	widget.Frame({-100, -100}, false);
	const float center = widget.Origin.x + widget.Width * 0.5f;
	const float barY = widget.Origin.y + 15.0f + ImGui::GetStyle().ItemSpacing.y;
	bool found = false;
	float displayedRed = 0.0f;
	const auto* data = ImGui::GetDrawData();
	for (int list = 0; list < data->CmdListsCount; ++list)
	{
		const auto& vertices = data->CmdLists[list]->VtxBuffer;
		for (int i = 0; i + 3 < vertices.Size; ++i)
		{
			const auto& a = vertices[i];
			const auto& b = vertices[i + 1];
			if (a.pos.y != barY || b.pos.y != barY || !(a.pos.x <= center && center < b.pos.x))
				continue;
			const float t = (center - a.pos.x) / (b.pos.x - a.pos.x);
			displayedRed = ImGui::ColorConvertU32ToFloat4(a.col).x * (1 - t) + ImGui::ColorConvertU32ToFloat4(b.col).x * t;
			found = true;
		}
	}
	CHECK(found);
	CHECK(std::abs(displayedRed - widget.State.GetCombinedColor(0.5f)[0]) <= 1.0f / 255.0f);
	return true;
}

int main()
{
	for (bool color : {false, true})
	{
		if (!Dragging(color) || !DisabledAndClamping(color) || !Insertion(color))
			return 1;
	}
	return WidthAndHdr() ? 0 : 1;
}
