#include "Scene.h"

void Scene::AddModel(Model0& model)
{
	_models.push_back(model);
	_gpStates.push_back(NULL);
}
