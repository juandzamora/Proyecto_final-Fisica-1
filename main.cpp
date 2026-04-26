#include <raylib.h>
#include <rlImGui.h>
#include <imgui.h>

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "window");

    Vector2 ballPosition = { (float)screenWidth/2, (float)screenHeight/2 };

    SetTargetFPS(60);
	
	//rlImGui setup
	rlImGuiSetup(true); 

    while (!WindowShouldClose())
	{
		if (IsKeyDown(KEY_RIGHT)) ballPosition.x += 2.0f;
		if (IsKeyDown(KEY_LEFT)) ballPosition.x -= 2.0f;
		if (IsKeyDown(KEY_UP)) ballPosition.y -= 2.0f;
		if (IsKeyDown(KEY_DOWN)) ballPosition.y += 2.0f;
		
		BeginDrawing();
		rlImGuiBegin();

		ImGui::Begin("Pelota config");
		ImGui::TextWrapped("Mover el circulo desde aqui");
		ImGui::DragFloat("Mover en x", &ballPosition.x);
		ImGui::DragFloat("Mover en y", &ballPosition.y);
		ImGui::End();

		ClearBackground(RAYWHITE);
		DrawText("move the ball with arrow keys", 10, 10, 20, DARKGRAY);

		DrawCircleV(ballPosition, 50, MAROON);

		//Renderizar en pantalla
		rlImGuiEnd();
		EndDrawing();
    }

	rlImGuiShutdown();
    CloseWindow();        // Close window and OpenGL context

    return 0;
}