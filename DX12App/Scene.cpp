#include "Scene.h"

void Scene::AddModel(std::unique_ptr<Model0>&& model)
{
	_models.push_back(std::move(model));
	_gpStates.push_back(NULL);
}
