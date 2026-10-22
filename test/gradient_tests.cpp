#include <ImGradientHDR.h>

#include <cmath>
#include <cstdio>

int main()
{
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
