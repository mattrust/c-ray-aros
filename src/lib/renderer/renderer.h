//
//  renderer.h
//  C-ray
//
//  Created by Valtteri Koskivuori on 19/02/2017.
//  Copyright © 2017-2022 Valtteri Koskivuori. All rights reserved.
//

#pragma once

#include <c-ray/c-ray.h>
#include "../datatypes/tile.h"
#include "../../common/timer.h"
#include "../../common/platform/thread.h"
#include "../protocol/server.h"

struct worker {
	struct cr_thread thread;
	bool thread_complete;
	
	bool paused; //SDL listens for P key pressed, which sets these, one for each thread.
	
	//Share info about the current tile with main thread
	struct tile_set *tiles;
	struct render_tile *currentTile;
	uint64_t totalSamples;
	
	long avg_per_sample_us; //Single tile pass

	struct camera *cam;
	struct renderer *renderer;
	struct texture *output;
	struct texture *buf;
	struct render_client *client; // Optional
};
typedef struct worker worker;
dyn_array_def(worker);

/// Renderer state data
struct state {
	size_t finishedPasses; // For interactive mode
	bool rendering;
	bool render_aborted; //SDL listens for X key pressed, which sets this
	bool saveImage;
	struct worker_arr workers;
	struct render_client_arr clients;
	struct cr_renderer_callbacks cb;
};

/// Preferences data (Set by user)
struct prefs {
	enum render_order tileOrder;
	
	size_t threads; //Amount of threads to render with
	bool fromSystem; //Did we ask the system for thread count
	size_t sampleCount;
	size_t bounces;
	unsigned tileWidth;
	unsigned tileHeight;
	
	//Output prefs
	unsigned override_width;
	unsigned override_height;
	size_t selected_camera;
	char *imgFilePath;
	char *imgFileName;
	size_t imgCount;
	char *node_list;
	bool iterative;
};

struct renderer {
	struct world *scene; //Scene to render
	struct state state;  //Internal state
	struct prefs prefs;  //User prefs
};

struct renderer *renderer_new(void);
struct texture *renderer_render(struct renderer *r);
void renderer_destroy(struct renderer *r);

struct prefs default_prefs(); // TODO: Remove