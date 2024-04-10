#define is_down(b) input->buttons[b].is_down
#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)

float player_1_px = 80, player_2_px = -80;
float player_1_dpx, player_1_dpy, player_1_py, player_2_py, player_2_dpx, player_2_dpy;
float arena_half_size_x = 90, arena_half_size_y = 47;
float player_half_size_x = 2.5, player_half_size_y = 12;
float ball_p_x, ball_p_y, ball_dp_x = 130, ball_dp_y = 0, ball_half_size = 1;

int player_1_score, player_2_score;
int player_1_life = 5 , player_2_life = 5;
int highScore = 0;
internal void simulate_player(float* px, float* py, float* dpx, float* dpy, float ddpx, float ddpy, float dt) {
	//update acceleration
	ddpx -= *dpx * 10.f;
	ddpy -= *dpy * 10.f;

	// Update position
	*px += *dpx * dt + ddpx * dt * dt * 0.5f;
	*py += *dpy * dt + ddpy * dt * dt * 0.5f;

	// Update velocity
	*dpx += ddpx * dt;
	*dpy += ddpy * dt;

	// Check arena bounds
	if (*px + player_half_size_x > arena_half_size_x) {
		*px = arena_half_size_x - player_half_size_x;
		*dpx = 0; // Stop horizontal movement
	}
	if (*px - player_half_size_x < -arena_half_size_x) {
		*px = -arena_half_size_x + player_half_size_x;
		*dpx = 0; // Stop horizontal movement
	}
	if (*py + player_half_size_y > arena_half_size_y) {
		*py = arena_half_size_y - player_half_size_y;
		*dpy = 0; // Stop vertical movement
	}
	if (*py - player_half_size_y < -arena_half_size_y) {
		*py = -arena_half_size_y + player_half_size_y;
		*dpy = 0; // Stop vertical movement
	}
}



internal bool aabb_vs_aabb(float p1x, float p1y, float hs1x, float hs1y, float p2x, float p2y, float hs2x, float hs2y) {
	return (p1x + hs1x > p2x - hs2x && p1x - hs1x < p2x + hs2x && p1y + hs1y > p2y - hs2y && p1y - hs1y < p2y + hs2y);
}

enum Gamemode {
	GM_MENU,
	GM_GAMEPLAY,
	GM_OVER
};

Gamemode current_gamemode = GM_MENU;
bool hot_button;
bool enemy_is_ai;

internal void simulate_game(Input* input, float dt) {
	draw_rect(0, 0, arena_half_size_x + 1, arena_half_size_y + 1, 0x000000);
	draw_rect(0, 0, 1, arena_half_size_y + 1, 0x555555);
	draw_arena_borders(arena_half_size_x, arena_half_size_y, 0xffffff);
	draw_arena_borders(arena_half_size_x + 1, arena_half_size_y + 1, 0x000000);

	if (pressed(BUTTON_ESC)) running = false;

	if (current_gamemode == GM_GAMEPLAY) {
		float player_1_ddpy = 0.f;
		float player_1_ddpx = 0.f;
		if (is_down(BUTTON_UP)) player_1_ddpy += 2000;
		if (is_down(BUTTON_DOWN)) player_1_ddpy -= 2000;
		if ((player_1_px < arena_half_size_x)) {
			if (is_down(BUTTON_RIGHT)) player_1_ddpx += 2000; // Movement to right
		} 
		if ((player_1_px > arena_half_size_x / 2)) {
			if (is_down(BUTTON_LEFT)) player_1_ddpx -= 2000;  // Movement to left
		}
		
		float player_2_ddpy = 0.f;
		float player_2_ddpx = 0.f;
		if (!enemy_is_ai) {
			if (is_down(BUTTON_W)) player_2_ddpy += 2000;
			if (is_down(BUTTON_S)) player_2_ddpy -= 2000;
			if (player_2_px < arena_half_size_x) {
				if (is_down(BUTTON_A)) player_2_ddpx -= 2000;  // Movement to left
				if (is_down(BUTTON_D)) player_2_ddpx += 2000;  // Movement to right
			}
		}
		else {
			player_2_ddpy = (ball_p_y - player_2_py) * 100;
			if (player_2_ddpy > 1000) player_2_ddpy = 1000;
			if (player_2_ddpy < -1000) player_2_ddpy = -1000;
		}

		// Simulate Ball
		{
			ball_p_x += ball_dp_x * dt;
			ball_p_y += ball_dp_y * dt;

			// Collision with players
			if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, player_1_px, player_1_py, player_half_size_x, player_half_size_y)) {
				ball_p_x = player_1_px - player_half_size_x - ball_half_size;
				ball_dp_x *= -1;
				player_2_score++;
				ball_dp_y = (ball_p_y - player_1_py) * 2 + player_1_dpy * 0.75f;
				player_1_dpx = 0;
			}
			else if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, player_2_px, player_2_py, player_half_size_x, player_half_size_y)) {
				ball_p_x = player_2_px + player_half_size_x + ball_half_size;
				ball_dp_x *= -1;
				player_1_score++;
				ball_dp_y = (ball_p_y - player_2_py) * 2 + player_2_dpy * 0.75f;
				player_2_dpx = 0;
			}

			// Collision with arena borders
			if (ball_p_y + ball_half_size > arena_half_size_y || ball_p_y - ball_half_size < -arena_half_size_y) {
				ball_dp_y *= -1;
			}

			if (ball_p_x + ball_half_size > arena_half_size_x) {
				ball_dp_x *= -1;
				ball_p_x = 0;
				ball_p_y = 0;
				ball_dp_y = 0;
				player_2_life--;
			}
			else if (ball_p_x - ball_half_size < -arena_half_size_x) {
				ball_dp_x *= -1;
				ball_p_x = 0;
				ball_p_y = 0;
				ball_dp_y = 0;
				player_1_life--;
			}
		}
		//simualte player
		simulate_player(&player_1_px, &player_1_py, &player_1_dpx, &player_1_dpy, player_1_ddpx, player_1_ddpy, dt);
		simulate_player(&player_2_px, &player_2_py, &player_2_dpx, &player_2_dpy, player_2_ddpx, player_2_ddpy, dt);
		draw_text(enemy_is_ai ? "AI LIFE " : "PLAYER I / LIFE ", -80, 40, .5f, 0xffffff);
		draw_number(player_1_life, enemy_is_ai ? -56 : - 32, 39, 1.f, 0xff0000);
		draw_number(player_1_score, -10, 40, 1.f, 0xbbffbb);
		draw_text("PLAYER II / LIFE ", 20, 40, .5f, 0xffffff);
		draw_number(player_2_life, 71, 39, 1.f, 0xff0000);
		draw_number(player_2_score, 10, 40, 1.f, 0xbbffbb);

		// Rendering
		draw_rect(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 0xffffff);
		draw_rect(player_1_px, player_1_py, player_half_size_x, player_half_size_y, 0x0000ff);
		draw_rect(player_2_px, player_2_py, player_half_size_x, player_half_size_y, 0xff0000);

		// Check for winner
		if (player_1_life <= 0 || player_2_life <= 0) {
			ball_p_x = 0;
			ball_p_y = 0;
			ball_dp_x = 130;
			ball_dp_y = 0;
			player_1_py = 0;
			player_1_dpy = 0;
			player_2_py = 0;
			player_2_dpy = 0;
			if (!enemy_is_ai) highScore = max(max(player_1_score, player_2_score), highScore);
			else highScore = max(player_2_score, highScore);
			current_gamemode = GM_OVER;
		}
	}
	else if (current_gamemode == GM_MENU) {
		if (pressed(BUTTON_LEFT) || pressed(BUTTON_RIGHT)) {
			hot_button = !hot_button;
		}

		if (pressed(BUTTON_ENTER)) {
			current_gamemode = GM_GAMEPLAY;
			enemy_is_ai = !hot_button;
		}

		draw_text("THE PONG GAME", -73, 40, 2, 0xff0000);
		draw_text("DEVELOPED USING CPP AND WINDOWSAPI", -73, 22, .75, 0xffffff);
		draw_text("EXTENDED JP CREATION", -71, 15, 1.22, 0x0000ff);
		
		draw_text(!hot_button ? "> SINGLE PLAYER" : "SINGLE PLAYER", -80, -10, hot_button ? 1 : 0.5, hot_button ? 0xaaaaaa : 0x00ff00);
		draw_text(hot_button ? "> MULTIPLAYER" : "MULTIPLAYER", 20, -10, hot_button ? 0.5 : 1, hot_button ? 0x00ff00 : 0xaaaaaa);
	}
	else {
		if (pressed(BUTTON_LEFT) || pressed(BUTTON_RIGHT)) {
			hot_button = !hot_button;
		}

		if (pressed(BUTTON_ENTER)) {
			if (hot_button) {
				exit(0);
			}
			player_1_score = 0;
			player_2_score = 0;
			player_1_life = 5;
			player_2_life = 5;
			current_gamemode = GM_MENU;
		}

		if (player_2_life <= 0)
			draw_text((enemy_is_ai) ? "OUR SYSTEM WON THE MATCH" : "PLAYER I WON THE MATCH", -73, 0, 1.1, 0xff0000);
		else
			draw_text((enemy_is_ai) ? "   YOU WON THE MATCH" : "PLAYER II WON THE MATCH", -73, 0, 1.1, 0xff0000);
		draw_text("HIGH SCORE ", -70, 15, .7, 0xffffff);
		draw_number(highScore, -25, 13, 1, 0xffffff);
		draw_text("YOUR SCORE ", 10, 15, .7, 0xffffff);
		draw_number((enemy_is_ai) ? player_2_score :max(player_1_score,player_2_score), 55, 13, 1, 0xffffff);
		draw_text("THE PONG GAME", -33, 40, 2 * 0.5, 0xff0000);
		draw_text("DEVELOPED USING CPP AND WINDOWSAPI", -33, 30, 0.75 * 0.5, 0xffffff);
		draw_text("EXTENDED JP CREATION", -31, 25, 1.22 * 0.5, 0x0000ff);

		draw_text(!hot_button ? "> MENU" : "MENU", -60, -20, hot_button ? 0.5 : 1, hot_button ? 0xaaaaaa : 0x00ff00);
		draw_text(hot_button ? "> EXIT" : "EXIT", 30, -20, hot_button ? 1 : 0.5, hot_button ? 0x00ff00 : 0xaaaaaa);
	}
}
