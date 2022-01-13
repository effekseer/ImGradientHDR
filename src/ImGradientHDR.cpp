#include "ImGradientHDR.h"

#include <imgui.h>
#include <vector>
#include <algorithm>
#include <string>
#include<cmath>

namespace
{
	template<typename T>
	void AddMarker(std::array<T, MarkerMax>& a, int32_t& count, T value)
	{
		const auto lb = std::lower_bound(
			a.begin(),
			a.begin() + count,
			value,
			[&](const T& a, const T& b) -> bool { return a.Position < b.Position; });

		if (lb != a.end())
		{
			const auto ind = lb - a.begin();
			std::copy(a.begin() + ind, a.begin() + count, a.begin() + ind + 1);
			*(a.begin() + ind) = value;
			count++;
		}
	}

	void DrawMarker(const ImVec2& pmin, const ImVec2& pmax, const ImU32& color, bool isSelected)
	{
		auto drawList = ImGui::GetWindowDrawList();
		const auto w = pmax.x - pmin.x;
		const auto h = pmax.y - pmin.y;
		const auto sign = std::signbit(h) ? -1 : 1;

		const auto margin = 2;
		const auto marginh = margin * sign;
		const auto outlineColor = isSelected ? ImGui::ColorConvertFloat4ToU32({ 0.0f, 0.0f, 1.0f, 1.0f }) : ImGui::ColorConvertFloat4ToU32({ 0.2f, 0.2f, 0.2f, 1.0f });

		drawList->AddTriangleFilled(
			{ pmin.x + w / 2, pmin.y },
			{ pmin.x + 0, pmin.y + h / 2 },
			{ pmin.x + w, pmin.y + h / 2 },
			outlineColor);

		drawList->AddRectFilled({ pmin.x + 0, pmin.y + h / 2 }, { pmin.x + w, pmin.y + h }, outlineColor);

		drawList->AddTriangleFilled(
			{ pmin.x + w / 2, pmin.y + marginh },
			{ pmin.x + 0 + margin, pmin.y + h / 2 },
			{ pmin.x + w - margin, pmin.y + h / 2 },
			color);

		drawList->AddRectFilled({ pmin.x + 0 + margin, pmin.y + h / 2 - sign }, { pmin.x + w - margin, pmin.y + h - marginh }, color);
	};

	template<typename T>
	void SortMarkers(std::array<T, MarkerMax>& a, int32_t& count, int32_t& selectedIndex, int32_t& draggingIndex)
	{
		struct SortedMarker
		{
			int index;
			T marker;
		};

		std::vector<SortedMarker> sortedMarker;

		for (int32_t i = 0; i < count; i++)
		{
			sortedMarker.emplace_back(SortedMarker{ i, a[i] });
		}

		std::sort(sortedMarker.begin(), sortedMarker.end(), [](const SortedMarker& a, const SortedMarker& b) { return a.marker.Position < b.marker.Position; });

		for (int32_t i = 0; i < count; i++)
		{
			a[i] = sortedMarker[i].marker;
		}

		if (selectedIndex != -1)
		{
			for (int32_t i = 0; i < count; i++)
			{
				if (sortedMarker[i].index == selectedIndex)
				{
					selectedIndex = i;
					break;
				}
			}
		}

		if (draggingIndex != -1)
		{
			for (int32_t i = 0; i < count; i++)
			{
				if (sortedMarker[i].index == draggingIndex)
				{
					draggingIndex = i;
					break;
				}
			}
		}
	}
}

ImGradientHDRState::ColorMarker* ImGradientHDRState::GetColorMarker(int32_t index)
{
	if (index < 0 ||
		index >= ColorCount)
	{
		return nullptr;
	}

	return &(Colors[index]);
}

ImGradientHDRState::AlphaMarker* ImGradientHDRState::GetAlphaMarker(int32_t index)
{
	if (index < 0 ||
		index >= AlphaCount)
	{
		return nullptr;
	}

	return &(Alphas[index]);
}

bool ImGradientHDRState::AddColorMarker(float x, std::array<float, 3> color, float intencity)
{
	if (ColorCount >= MarkerMax)
	{
		return false;
	}

	x = std::max(std::min(x, 1.0f), 0.0f);

	const auto marker = ColorMarker{ x, color, intencity };
	AddMarker(Colors, ColorCount, marker);
	return true;
}

bool ImGradientHDRState::AddAlphaMarker(float x, float alpha)
{
	if (AlphaCount >= MarkerMax)
	{
		return false;
	}

	x = std::max(std::min(x, 1.0f), 0.0f);

	const auto marker = AlphaMarker{ x, alpha };

	AddMarker(Alphas, AlphaCount, marker);
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
			return Alphas[0].Alpha;
		}

		if (Alphas[AlphaCount - 1].Position <= x)
		{
			return Alphas[AlphaCount - 1].Alpha;
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

bool ImGradientHDR(int32_t gradientID, ImGradientHDRState& state, ImGradientHDRTemporaryState& temporaryState)
{
	ImGui::PushID(gradientID);

	auto originPos = ImGui::GetCursorScreenPos();

	auto drawList = ImGui::GetWindowDrawList();

	const auto margin = 5;

	const auto width = ImGui::GetContentRegionAvail().x - margin * 2;
	const auto barHeight = 20;
	const auto markerWidth = 10;
	const auto markerHeight = 15;

	for (int i = 0; i < state.AlphaCount; i++)
	{
		const auto x = (int)(state.Alphas[i].Position * width);
		const auto c = state.Alphas[i].Alpha;
		ImGui::SetCursorScreenPos({ originPos.x + x - 5, originPos.y });

		DrawMarker(
			{ originPos.x + x - 5, originPos.y + markerHeight },
			{ originPos.x + x + 5, originPos.y + 0 },
			ImGui::ColorConvertFloat4ToU32({ c, c, c, 1.0f }),
			temporaryState.selectedMarkerType == ImGradientHDRMarkerType::Alpha && temporaryState.selectedIndex == i);

		if (ImGui::InvisibleButton(("a" + std::to_string(i)).c_str(), { markerWidth, markerHeight }))
		{
			temporaryState.selectedMarkerType = ImGradientHDRMarkerType::Alpha;
			temporaryState.selectedIndex = i;
		}

		if (temporaryState.draggingIndex == -1 && ImGui::IsItemHovered() && ImGui::IsMouseDown(0))
		{
			temporaryState.draggingMarkerType = ImGradientHDRMarkerType::Alpha;
			temporaryState.draggingIndex = i;
		}

		if (!ImGui::IsMouseDown(0))
		{
			temporaryState.draggingIndex = -1;
			temporaryState.draggingMarkerType = ImGradientHDRMarkerType::Unknown;
		}

		if (temporaryState.draggingMarkerType == ImGradientHDRMarkerType::Alpha && temporaryState.draggingIndex == i && ImGui::IsMouseDragging(0))
		{
			const auto diff = ImGui::GetIO().MouseDelta.x / width;
			state.Alphas[i].Position += diff;
			state.Alphas[i].Position = std::max(std::min(state.Alphas[i].Position, 1.0f), 0.0f);
		}
	}

	if (temporaryState.draggingMarkerType == ImGradientHDRMarkerType::Alpha)
	{
		SortMarkers(state.Alphas, state.AlphaCount, temporaryState.selectedIndex, temporaryState.draggingIndex);
	}

	ImGui::SetCursorScreenPos(originPos);

	ImGui::InvisibleButton("AlphaArea", { width,markerHeight });

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
	{
		float x = (ImGui::GetIO().MousePos.x - originPos.x) / width;
		const auto c = state.GetColor(x);
		state.AddAlphaMarker(x, c[3]);
	}

	originPos = ImGui::GetCursorScreenPos();

	ImGui::InvisibleButton("BarArea", { width,barHeight });

	const int32_t gridSize = 10;

	drawList->AddRectFilled(ImVec2(originPos.x - 2, originPos.y - 2),
		ImVec2(originPos.x + width + 2, originPos.y + barHeight + 2),
		IM_COL32(100, 100, 100, 255));

	for (int y = 0; y * gridSize < barHeight; y += 1)
	{
		for (int x = 0; x * gridSize < width; x += 1)
		{
			int wgrid = std::min(gridSize, static_cast<int>(width) - x * gridSize);
			int hgrid = std::min(gridSize, barHeight - y * gridSize);
			ImU32 color = IM_COL32(100, 100, 100, 255);

			if ((x + y) % 2 == 0)
			{
				color = IM_COL32(50, 50, 50, 255);
			}

			drawList->AddRectFilled(ImVec2(originPos.x + x * gridSize, originPos.y + y * gridSize),
				ImVec2(originPos.x + x * gridSize + wgrid, originPos.y + y * gridSize + hgrid),
				color);
		}
	}

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
				ImVec2(originPos.x + xkeys[i + 1] * width, originPos.y + barHeight),
				colorAU32, colorBU32, colorBU32, colorAU32);
		}
	}

	originPos = ImGui::GetCursorScreenPos();

	for (int i = 0; i < state.ColorCount; i++)
	{
		const auto x = (int)(state.Colors[i].Position * width);
		const auto c = state.Colors[i].Color;

		DrawMarker(
			{ originPos.x + x - 5, originPos.y + 0 },
			{ originPos.x + x + 5, originPos.y + markerHeight },
			ImGui::ColorConvertFloat4ToU32({ c[0], c[1], c[2], 1.0f }),
			temporaryState.selectedMarkerType == ImGradientHDRMarkerType::Color && temporaryState.selectedIndex == i);

		ImGui::SetCursorScreenPos({ originPos.x + x - 5, originPos.y });
		if (ImGui::InvisibleButton(("c" + std::to_string(i)).c_str(), { markerWidth, markerHeight }))
		{
			temporaryState.selectedMarkerType = ImGradientHDRMarkerType::Color;
			temporaryState.selectedIndex = i;
		}

		if (temporaryState.draggingIndex == -1 && ImGui::IsItemHovered() && ImGui::IsMouseDown(0))
		{
			temporaryState.draggingMarkerType = ImGradientHDRMarkerType::Color;
			temporaryState.draggingIndex = i;
		}

		if (!ImGui::IsMouseDown(0))
		{
			temporaryState.draggingIndex = -1;
			temporaryState.draggingMarkerType = ImGradientHDRMarkerType::Unknown;
		}

		if (temporaryState.draggingMarkerType == ImGradientHDRMarkerType::Color && temporaryState.draggingIndex == i && ImGui::IsMouseDragging(0))
		{
			const auto diff = ImGui::GetIO().MouseDelta.x / width;
			state.Colors[i].Position += diff;
			state.Colors[i].Position = std::max(std::min(state.Colors[i].Position, 1.0f), 0.0f);
		}
	}

	if (temporaryState.draggingMarkerType == ImGradientHDRMarkerType::Color)
	{
		SortMarkers(state.Colors, state.ColorCount, temporaryState.selectedIndex, temporaryState.draggingIndex);
	}

	ImGui::SetCursorScreenPos(originPos);

	ImGui::InvisibleButton("ColorArea", { width,markerHeight });

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
	{
		float x = (ImGui::GetIO().MousePos.x - originPos.x) / width;
		const auto c = state.GetColor(x);
		state.AddColorMarker(x, { c[0], c[1], c[2] }, 1.0f);
	}

	ImGui::PopID();

	return true;
}
