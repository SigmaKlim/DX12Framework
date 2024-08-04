#pragma once
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

struct Camera
{
	Vector3 eyePos	= { 0.0f, 0.0f, -10.0f };
	Vector3 target	= { 0.0f, 0.0f, 0.0f };
	Vector3 up		= { 0.0f, 1.0f, 0.0f };

	void MakeViewMatrix(Matrix& view)
	{
		view = Matrix::CreateLookAt(eyePos, target, up);
	}
};

