#pragma once
#include <vector>
#include "Model.h"

struct Scene
{
	void AddModel(std::unique_ptr<Model0>&& model);
	
private:
	std::vector<std::unique_ptr<Model0>> _models;
	std::vector<ComPtr<ID3D12PipelineState>> _gpStates; //very bad, see source.cpp TODO for improvement
	
	friend class Render;
};