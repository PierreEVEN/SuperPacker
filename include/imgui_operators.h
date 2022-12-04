#pragma once

#include <imgui.h>

inline ImVec2 operator+(const ImVec2& left, const ImVec2& right)
{
	return { left.x + right.x, left.y + right.y };
}

inline ImVec2 operator+(const ImVec2& left, const float& right)
{
	return { left.x + right, left.y + right };
}

inline ImVec2 operator-(const ImVec2& left, const ImVec2& right)
{
	return { left.x - right.x, left.y - right.y };
}

inline ImVec2 operator-(const ImVec2& left, const float& right)
{
	return { left.x - right, left.y - right };
}

inline ImVec2 operator*(const ImVec2& left, const ImVec2& right)
{
	return { left.x * right.x, left.y * right.y };
}


inline ImVec2 operator*(const ImVec2& left, const float& right)
{
	return { left.x * right, left.y * right };
}

inline ImVec2 operator/(const ImVec2& left, const float& right)
{
	return { left.x / right, left.y / right };
}

inline ImVec2 operator/(const ImVec2& left, const ImVec2& right)
{
	return { left.x / right.x, left.y / right.y };
}

inline ImVec2& operator+=(ImVec2& left, const ImVec2& right)
{
	left.x += right.x;
	left.y += right.y;
	return left;
}

