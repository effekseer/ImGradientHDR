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

	bool AddColorMarker(float x, std::array<float, 3> color, float intensity);

	std::array<float, 4> GetColor(float x) const;
};

bool ImGradientHDR(ImGradientHDRState& state);
