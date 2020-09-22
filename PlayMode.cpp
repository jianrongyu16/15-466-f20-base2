#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint area_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > area_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("area.pnct"));
	area_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > area_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("area.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = area_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = area_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

PlayMode::PlayMode() : scene(*area_scene) {
	cubes.resize(4);
	for (auto &transform : scene.transforms) {

		if (transform.name == "Cube") cubes[0] = &transform;
		else if (transform.name == "Cube.001") cubes[1] = &transform;
        else if (transform.name == "Cube.002") cubes[2] = &transform;
        else if (transform.name == "Cube.003") cubes[3] = &transform;
	}

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
    for (auto c: cubes) {
        c->position.x = 0;
        c->position.y = 0;
    }
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			ak.downs += 1;
			ak.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			dk.downs += 1;
			dk.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			wk.downs += 1;
			wk.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			sk.downs += 1;
			sk.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_LEFT) {
            left.downs += 1;
            left.pressed = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_RIGHT) {
            right.downs += 1;
            right.pressed = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_UP) {
            up.downs += 1;
            up.pressed = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_DOWN) {
            down.downs += 1;
            down.pressed = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_q) {
            qk.pressed = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_e) {
            ek.pressed = true;
            return true;
        }
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			ak.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			dk.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			wk.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			sk.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_LEFT) {
            left.pressed = false;
            return true;
        } else if (evt.key.keysym.sym == SDLK_RIGHT) {
            right.pressed = false;
            return true;
        } else if (evt.key.keysym.sym == SDLK_UP) {
            up.pressed = false;
            return true;
        } else if (evt.key.keysym.sym == SDLK_DOWN) {
            down.pressed = false;
            return true;
        } else if (evt.key.keysym.sym == SDLK_q) {
            qk.pressed = false;
            cub_change=-1;
            return true;
        } else if (evt.key.keysym.sym == SDLK_e) {
            ek.pressed = false;
            cub_change=1;
            return true;
        }
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			);
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
    if (dead - elapsed > 0) {
        dead -= elapsed;
        return;
    } else if (dead>0) {
        for (auto c: cubes) {
            c->position.x = 0;
            c->position.y = 0;
        }
        dead = 0;
        timer = 4.0f;
        score = 0;
        cub = 1;
        for (auto cc: cubes) {
            cc->scale.x=1;
            cc->scale.y=1;
            cc->scale.z=1;
        }
    } else {
        timer-=elapsed;
    }

    cub=((cub+cub_change)%4+4)%4;
    cub_change = 0;
    cubes[cub]->scale.x=1;
    cubes[cub]->scale.y=1;
    cubes[cub]->scale.z=1;
	if (right.pressed||left.pressed||down.pressed||up.pressed) speed += elapsed / 10.0f;
	else speed = elapsed;

    auto calculate = [&](std::vector<Scene::Transform*> cs) {
        float s = 0.0f;
        for (int i = 1; i<=3; i++){
            s+=std::min(std::abs(cs[i]->position.x)+std::abs(cs[i]->position.y), 4.0f);
        }
        return s;
    };

    auto decide_death = [this](std::vector<Scene::Transform*> cs) {

        if (std::abs(cubes[cub]->position.x) >= 2 || std::abs(cubes[cub]->position.y) >= 2 || timer<=0) {
            dead = 2.0f;
        }
        if (timer<=0) {
            best_score = std::max(score, best_score);
        }
    };


    decide_death(cubes);

    score = calculate(cubes);
    if (right.pressed) {
        cubes[cub]->position.x+=speed;
    }
    if (left.pressed) {
        cubes[cub]->position.x-=speed;
    }
    if (down.pressed) {
        cubes[cub]->position.y-=speed;
    }
    if (up.pressed) {
        cubes[cub]->position.y+=speed;
    }
    if (qk.pressed) {
        cubes[((cub-1)%4+4)%4]->scale.x=1.2;
        cubes[((cub-1)%4+4)%4]->scale.y=1.2;
        cubes[((cub-1)%4+4)%4]->scale.z=1.2;
    }
    if (ek.pressed) {
        cubes[(cub+1)%4]->scale.x=1.2;
        cubes[(cub+1)%4]->scale.y=1.2;
        cubes[(cub+1)%4]->scale.z=1.2;
    }

	{

		//combine inputs into a move:
		constexpr float PlayerSpeed = 30.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (ak.pressed && !dk.pressed) move.x =-1.0f;
		if (!ak.pressed && dk.pressed) move.x = 1.0f;
		if (sk.pressed && !wk.pressed) move.y =-1.0f;
		if (!sk.pressed && wk.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 forward = -frame[2];

		camera->transform->position += move.x * right + move.y * forward;
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
    wk.downs = 0;
    ak.downs = 0;
    sk.downs = 0;
    dk.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	GL_ERRORS();
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0,0,-0.5)));
	GL_ERRORS();
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	GL_ERRORS();
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
        float ofs = 2.0f / drawable_size.y;
		std::string s = "time left: " + std::to_string(timer);
        std::string s1 = "score: " + std::to_string(score);
        std::string s2 = "best score: " + std::to_string(best_score);

        std::string s3 = timer>0?"OUT!!!":"Timed out";
        if (dead>0)lines.draw_text(s3,
                        glm::vec3(0, 0, 0.0),
                        glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                        glm::u8vec4(0xff, 0xff, 0xff, 0x00));
        lines.draw_text(s,
                        glm::vec3(0.8f * H +0.7f, 0.1f * H, 0.0),
                        glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                        glm::u8vec4(0xff, 0xff, 0xff, 0x00));
        lines.draw_text(s1,
                        glm::vec3(0.8f * H + 0.7f , 0.1f * H+0.1f, 0.0),
                        glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                        glm::u8vec4(0xff, 0xff, 0xff, 0x00));
        lines.draw_text(s2,
                        glm::vec3(0.8f * H + 0.7f , 0.1f * H+0.2f, 0.0),
                        glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                        glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));

		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
}
