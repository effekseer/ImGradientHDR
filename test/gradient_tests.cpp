#include <ImGradientHDR.h>

#include <cmath>
#include <cstdio>

int main()
{
	ImGradientHDRState inserted{};
	for (int i = MarkerMax - 1; i >= 0; --i)
	{
		const float x = static_cast<float>(i) / (MarkerMax - 1);
		if (!inserted.AddColorMarker(x, {x, 0, 0}, 1) || !inserted.AddAlphaMarker(x, x))
			return 1;
	}
	for (int i = 0; i < MarkerMax; ++i)
	{
		const float x = static_cast<float>(i) / (MarkerMax - 1);
		if (inserted.Colors[i].Position != x || inserted.Colors[i].Color[0] != x || inserted.Alphas[i].Position != x || inserted.Alphas[i].Alpha != x)
		{
			std::fprintf(stderr, "Inserting keys overwrote existing keys.\n");
			return 1;
		}
	}
	if (inserted.AddColorMarker(0.5f, {1, 1, 1}, 1) || inserted.AddAlphaMarker(0.5f, 1))
		return 1;

	ImGradientHDRState state;
	if (!state.AddColorMarker(1.0f, {0.0f, 0.0f, 1.0f}, 3.0f) ||
		!state.AddColorMarker(0.0f, {1.0f, 0.0f, 0.0f}, 1.0f) ||
		!state.AddAlphaMarker(1.0f, 1.0f) || !state.AddAlphaMarker(0.0f, 0.0f))
	{
		std::fprintf(stderr, "Failed to add gradient keys.\n");
		return 1;
	}

	const auto middle = state.GetCombinedColor(0.5f);
	const std::array<float, 4> expected = {1.0f, 0.0f, 1.0f, 0.5f};
	for (std::size_t i = 0; i < expected.size(); ++i)
	{
		if (std::abs(middle[i] - expected[i]) > 0.0001f)
		{
			std::fprintf(stderr, "HDR color/alpha interpolation differs at component %zu.\n", i);
			return 1;
		}
	}

	if (state.GetCombinedColor(-1.0f) != state.GetCombinedColor(0.0f) ||
		state.GetCombinedColor(2.0f) != state.GetCombinedColor(1.0f) ||
		!state.RemoveColorMarker(0) || !state.RemoveAlphaMarker(0) ||
		state.GetCombinedColor(0.0f) != std::array<float, 4>{0.0f, 0.0f, 3.0f, 1.0f})
	{
		std::fprintf(stderr, "Endpoint clamping or key removal failed.\n");
		return 1;
	}
	return 0;
}
