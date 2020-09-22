#include "Mode.hpp"

#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, wk, sk, dk, ak, qk, ek;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	std::vector<Scene::Transform*> cubes;
	float speed = 0.0f;
	float enlarge = 0.5f;
	uint32_t cub = 1;
    uint32_t cub_change = 0;
	float timer = 4.0f;
	float score = 0.0f;
    float best_score = 0.0f;
	float dead = 0.0f;
	//camera:
	Scene::Camera *camera = nullptr;
};
