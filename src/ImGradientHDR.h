#pragma once

#include <stdint.h>
#include <array>

const int32_t MarkerMax = 8;

struct ImGradientHDRState
{
	struct ColorMarker
	{
		float Position;
		std::array<float, 3> Color;
		float Intensity;
	};

	struct AlphaMarker
	{
		float Position;
		float Alpha;
	};

	int ColorCount = 0;
	int AlphaCount = 0;
	std::array<ColorMarker, MarkerMax> Colors;
	std::array<AlphaMarker, MarkerMax> Alphas;


	ColorMarker* GetColorMarker(int32_t index);

	AlphaMarker* GetAlphaMarker(int32_t index);

	bool AddColorMarker(float x, std::array<float, 3> color, float intensity);

	bool AddAlphaMarker(float x, float alpha);

	std::array<float, 4> GetColor(float x) const;
};

enum class ImGradientHDRMarkerType
{
	Color,
	Alpha,
	Unknown,
};

struct ImGradientHDRTemporaryState
{
	ImGradientHDRMarkerType selectedMarkerType = ImGradientHDRMarkerType::Unknown;
	int selectedIndex = -1;

	ImGradientHDRMarkerType draggingMarkerType = ImGradientHDRMarkerType::Unknown;
	int draggingIndex = -1;
};

bool ImGradientHDR(int32_t gradientID, ImGradientHDRState& state, ImGradientHDRTemporaryState& temporaryState);
