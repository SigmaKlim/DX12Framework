#pragma once
#include <vector>
#include "Model.h"

struct Scene
{
	void AddModel(Model0& model);
	
private:
	std::vector<Model0> _models;
	std::vector<ComPtr<ID3D12PipelineState>> _gpStates; //very bad, see source.cpp TODO for improvement
	
	friend class Render;
};