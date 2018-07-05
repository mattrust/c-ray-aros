//
//  ui.h
//  C-Ray
//
//  Created by Valtteri Koskivuori on 19/02/2017.
//  Copyright © 2017 Valtteri Koskivuori. All rights reserved.
//

#pragma once

struct renderer;

#ifdef UI_ENABLED
struct display {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	SDL_Texture *overlayTexture;
	bool isBorderless;
	bool isFullScreen;
	double windowScale;
};

int initSDL(struct renderer *r);
void drawWindow(struct renderer *r);
#endif

void printDuration(double time);
void getKeyboardInput(struct renderer *r);
