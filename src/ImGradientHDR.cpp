#include "ImGradientHDR.h"

#include <imgui.h>
#include <vector>
#include <algorithm>
#include <string>

bool ImGradientHDRState::AddColorMarker(float x, std::array<float, 3> color, float intencity)
{
	if (ColorCount >= MarkerMax)
	{
		return false;
	}

	const auto marker = ColorMarker{ x, color, intencity };

	const auto lb = std::lower_bound(
		Colors.begin(),
		Colors.begin() + ColorCount,
		marker,
		[&](const ColorMarker& a, const ColorMarker& b) -> bool { return a.Position < b.Position; });

	if (lb != Colors.end())
	{
		const auto ind = lb - Colors.begin();
		std::copy(Colors.begin() + ind, Colors.begin() + ColorCount, Colors.begin() + ind + 1);
		*(Colors.begin() + ind) = marker;
		ColorCount++;
	}

	return true;
}

std::array<float, 4> ImGradientHDRState::GetColor(float x) const
{
	auto getColor = [&](float x)
	{
		if (ColorCount == 0)
		{
			return std::array<float, 3>{1.0f, 1.0f, 1.0f};
		}

		if (x < Colors[0].Position)
		{
			return Colors[0].Color;
		}

		if (Colors[ColorCount - 1].Position <= x)
		{
			return Colors[ColorCount - 1].Color;
		}

		for (int i = 0; i < ColorCount - 1; i++)
		{
			if (Colors[i].Position <= x && x < Colors[i + 1].Position)
			{
				const auto area = Colors[i + 1].Position - Colors[i].Position;
				const auto alpha = (x - Colors[i].Position) / area;
				const auto r = Colors[i + 1].Color[0] * alpha + Colors[i].Color[0] * (1.0f - alpha);
				const auto g = Colors[i + 1].Color[1] * alpha + Colors[i].Color[1] * (1.0f - alpha);
				const auto b = Colors[i + 1].Color[2] * alpha + Colors[i].Color[2] * (1.0f - alpha);
				return std::array<float, 3>{ r, g, b };
			}
		}

		return std::array<float, 3>{1.0f, 1.0f, 1.0f};
	};

	auto getAlpha = [&](float x)
	{
		if (AlphaCount == 0)
		{
			return 1.0f;
		}

		if (x < Alphas[0].Position)
		{
			return 1.0f;
		}

		if (Alphas[ColorCount - 1].Position <= x)
		{
			return Alphas[ColorCount - 1].Alpha;
		}

		for (int i = 0; i < AlphaCount - 1; i++)
		{
			if (Alphas[i].Position <= x && x < Alphas[i + 1].Position)
			{
				const auto area = Alphas[i + 1].Position - Alphas[i].Position;
				const auto alpha = (x - Alphas[i].Position) / area;
				return Alphas[i + 1].Alpha * alpha + Alphas[i].Alpha * (1.0f - alpha);
			}
		}

		return 1.0f;
	};

	const auto c = getColor(x);
	return std::array<float, 4>{c[0], c[1], c[2], getAlpha(x)};
}

bool ImGradientHDR(int32_t gradientID, ImGradientHDRState& state, int& selectedIndex, int& draggingIndex)
{
	ImGui::PushID(gradientID);

	const auto originPos = ImGui::GetCursorScreenPos();

	auto drawList = ImGui::GetWindowDrawList();

	const auto margin = 5;
	const auto width = ImGui::GetContentRegionAvail().x - margin * 2;
	const int height = 20;

	ImGui::InvisibleButton("Bar", { width,height });

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
	{
		float x = (ImGui::GetIO().MousePos.x - originPos.x) / width;
		const auto c = state.GetColor(x);
		state.AddColorMarker(x, { c[0], c[1], c[2] }, 1.0f);
	}

	drawList->AddRectFilled(ImVec2(originPos.x - 2, originPos.y - 2),
		ImVec2(originPos.x + width + 2, originPos.y + height + 2),
		IM_COL32(100, 100, 100, 255));

	if (state.ColorCount == 0 && state.AlphaCount == 0)
	{

	}
	else
	{
		std::vector<float> xkeys;
		xkeys.reserve(16);

		for (int32_t i = 0; i < state.ColorCount; i++)
		{
			xkeys.emplace_back(state.Colors[i].Position);
		}

		for (int32_t i = 0; i < state.AlphaCount; i++)
		{
			xkeys.emplace_back(state.Alphas[i].Position);
		}

		xkeys.emplace_back(0.0f);
		xkeys.emplace_back(1.0f);

		auto result = std::unique(xkeys.begin(), xkeys.end());
		xkeys.erase(result, xkeys.end());

		std::sort(xkeys.begin(), xkeys.end());

		for (size_t i = 0; i < xkeys.size() - 1; i++)
		{
			const auto c1 = state.GetColor(xkeys[i]);
			const auto c2 = state.GetColor(xkeys[i + 1]);

			const auto colorAU32 = ImGui::ColorConvertFloat4ToU32({ c1[0], c1[1], c1[2], c1[3] });
			const auto colorBU32 = ImGui::ColorConvertFloat4ToU32({ c2[0], c2[1], c2[2], c2[3] });

			drawList->AddRectFilledMultiColor(ImVec2(originPos.x + xkeys[i] * width, originPos.y),
				ImVec2(originPos.x + xkeys[i + 1] * width, originPos.y + height),
				colorAU32, colorBU32, colorBU32, colorAU32);
		}
	}

	const auto barEndPos = ImGui::GetCursorScreenPos();

	for (int i = 0; i < state.ColorCount; i++)
	{
		// TODO move marker
		const auto x = (int)(state.Colors[i].Position * width);
		const auto c = state.Colors[i].Color;

		const auto drawMarker = [](const ImVec2& pmin, const ImVec2& pmax, const ImU32& color, bool isSelected)
		{
			auto drawList = ImGui::GetWindowDrawList();
			const auto w = pmax.x - pmin.x;
			const auto h = pmax.y - pmin.y;
			const auto margin = 2;
			const auto outlineColor = isSelected ? ImGui::ColorConvertFloat4ToU32({ 0.0f, 0.0f, 1.0f, 1.0f }) : ImGui::ColorConvertFloat4ToU32({ 0.2f, 0.2f, 0.2f, 1.0f });

			drawList->AddTriangleFilled(
				{ pmin.x + w / 2, pmin.y },
				{ pmin.x + 0, pmin.y + h / 2 },
				{ pmin.x + w, pmin.y + h / 2 },
				outlineColor);

			drawList->AddRectFilled({ pmin.x + 0, pmin.y + h / 2 }, { pmin.x + w, pmin.y + h }, outlineColor);

			drawList->AddTriangleFilled(
				{ pmin.x + w / 2, pmin.y + margin },
				{ pmin.x + 0 + margin, pmin.y + h / 2 },
				{ pmin.x + w - margin, pmin.y + h / 2 },
				color);

			drawList->AddRectFilled({ pmin.x + 0 + margin, pmin.y + h / 2 }, { pmin.x + w - margin, pmin.y + h - margin }, color);

		};

		drawMarker(
			{ originPos.x + x - 5, originPos.y + height },
			{ originPos.x + x + 5, originPos.y + height + 20 },
			ImGui::ColorConvertFloat4ToU32({ c[0], c[1], c[2], 1.0f }),
			selectedIndex == i);

		ImGui::SetCursorScreenPos({ originPos.x + x - 5, originPos.y + height });
		if (ImGui::InvisibleButton(("c" + std::to_string(i)).c_str(), { 10, 20 }))
		{
			selectedIndex = i;
		}

		if (draggingIndex == -1 && ImGui::IsItemHovered() && ImGui::IsMouseDown(0))
		{
			draggingIndex = i;
		}

		if (!ImGui::IsMouseDown(0))
		{
			draggingIndex = -1;
		}

		if (draggingIndex == i && ImGui::IsMouseDragging(0))
		{
			const auto diff = ImGui::GetIO().MouseDelta.x / width;
			state.Colors[i].Position += diff;
		}
	}

	for (int i = 0; i < state.AlphaCount; i++)
	{
		// TODO add draw marker
	}

	ImGui::SetCursorScreenPos(barEndPos);

	ImGui::PopID();

	return true;
}
