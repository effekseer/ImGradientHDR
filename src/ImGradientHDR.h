#pragma once

#include <stdint.h>
#include <array>

const int32_t MarkerMax = 8;

struct ImGradientHDRState
{
	int ColorCount = 0;
	int AlphaCount = 0;
	std::array<std::array<float, 3>, MarkerMax> Colors;
	std::array<float, MarkerMax> ColorPositions;
	std::array<float, MarkerMax> ColorIntensities;

	std::array<float, MarkerMax> Alphas;
	std::array<float, MarkerMax> AlphaPositions;

	std::array<float, 4> GetColor(float x) const;
};

bool ImGradientHDR(ImGradientHDRState& state);
