#include <cmath>
using std::sin;
using std::cos;
using std::acos;
using std::pow;
using std::sqrt;
#include <vector>


#include <raylib.h>
#include <raymath.h>

#include <rlImGui.h>
#include <imgui.h>

void drawBezierCurve(Vector3 inicio,Vector3 final,Vector3 control,Color color)
{
	Vector3 punto_actual = inicio;

	punto_actual.y = inicio.y;

	if(punto_actual.y != control.y && punto_actual.y != final.y)
		return;

	float tiempo = 0.0f;
	while(tiempo <= 1.0f)
	{
		punto_actual.x = pow((1 - tiempo), 2) * inicio.x + 2*(1-tiempo)*tiempo*control.x + pow(tiempo, 2)*final.x;
		punto_actual.z = pow((1 - tiempo), 2) * inicio.z + 2*(1-tiempo)*tiempo*control.z + pow(tiempo, 2)*final.z;

		DrawPoint3D(punto_actual, color);

		tiempo += 0.01f;
	}
}

void manageInput(Camera3D &camara, float &angulo_x, float &angulo_y)
{
	if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
	{
		Vector2 mouse_delta = GetMouseDelta();

		const float sensibilidadX = 0.0075;
		const float sensibilidadY = 0.0025;

		angulo_x += mouse_delta.x * sensibilidadX;
		angulo_y += mouse_delta.y * sensibilidadY;
			
		const float epsilon = 0.01;
		angulo_y = Clamp(angulo_y, 0 + epsilon, PI - epsilon);

		float hipotenusa = sqrt(pow(camara.position.x - camara.target.x, 2) + pow(camara.position.y - camara.target.y, 2) + pow(camara.position.z - camara.target.z, 2));

		camara.position = {
			(cos(angulo_x)*sin(angulo_y)*hipotenusa) + camara.target.x, 
			(cos(angulo_y)*hipotenusa) + camara.target.y,
			(sin(angulo_x)*sin(angulo_y)*hipotenusa) + camara.target.z
		};
	}

	if(IsKeyDown(KEY_W))
	{
		float hipotenusa = sqrt(pow(camara.position.x - camara.target.x, 2) + pow(camara.position.y - camara.target.y, 2) + pow(camara.position.z - camara.target.z, 2)) - 0.1f;

		//float cosX = cos(angulo_x);
		//float senY = sin(angulo_y);

		camara.position = {
			(cos(angulo_x)*sin(angulo_y)*hipotenusa) + camara.target.x, 
			(cos(angulo_y)*hipotenusa) + camara.target.y,
			(sin(angulo_x)*sin(angulo_y)*hipotenusa) + camara.target.z
		};
	}

	if(IsKeyDown(KEY_S))
	{
		float hipotenusa = sqrt(pow(camara.position.x - camara.target.x, 2) + pow(camara.position.y - camara.target.y, 2) + pow(camara.position.z - camara.target.z, 2)) + 0.1f;

		camara.position = {
			(cos(angulo_x)*sin(angulo_y)*hipotenusa) + camara.target.x, 
			(cos(angulo_y)*hipotenusa) + camara.target.y,
			(sin(angulo_x)*sin(angulo_y)*hipotenusa) + camara.target.z
		};
	}

	if(IsKeyDown(KEY_A))
	{
		camara.position.x += 0.1f;
		camara.target.x += 0.1f;
	}

	if(IsKeyDown(KEY_D))
	{
		camara.position.x -= 0.1f;
		camara.target.x -= 0.1f;
	}
}

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Proyecto final - Fisica 1");

	Camera3D camara = {0};

	//Posicion
	camara.position = {0.0f, 10.0f, 10.0f};
	//Objetivo (a donde esta mirando)
	camara.target = { 0.0f, 0.0f, 0.0f };

	camara.up = { 0.0f, 1.0f, 0.0f }; 
	
	camara.fovy = 45.0f;
	camara.projection = CAMERA_PERSPECTIVE;

    Vector3 ballPosition = { 0.0f, 0.0f, 0.0f};

	const int max_fps = 60;

    SetTargetFPS(max_fps);
	
	//rlImGui setup
	rlImGuiSetup(true); 
	ImGuiIO& io = ImGui::GetIO();

	bool boton_abajo = false;

	float angulo_y = 0.0f;
	float angulo_x = 0.0f;

	{
		float hipotenusa = sqrt(pow(camara.position.x - camara.target.x, 2) + pow(camara.position.y - camara.target.y, 2) + pow(camara.position.z - camara.target.z, 2));
		angulo_y = acos(camara.position.y/hipotenusa);
		angulo_x = acos(camara.position.x/(sin(angulo_y) * hipotenusa));
	}

	float angulo_bola = 0.0f;
	std::vector<Vector3> puntos_tierra;
	bool not_full_circle = true;

    while (!WindowShouldClose())
	{	
		//input
		if(!io.WantCaptureKeyboard)
			manageInput(camara, angulo_x, angulo_y);

		BeginDrawing();
		rlImGuiBegin();

		ClearBackground(BLACK); 
		BeginMode3D(camara);

		float suma_ambos = 0;

		//Mover la pelota
		ImGui::Begin("Planeta config");
		ImGui::TextWrapped("config");
		ImGui::DragFloat("Mover en z", &ballPosition.y, 0.01);
		ImGui::DragFloat("Ambas", &suma_ambos);
		ImGui::End();

		camara.position += {suma_ambos, suma_ambos, 0};

		DrawSphere(ballPosition, 1, YELLOW);
		const float distancia_tierra = 3;

		Vector3 tierra_posicion = {cos(angulo_bola)*distancia_tierra, ballPosition.y,  sin(angulo_bola)*distancia_tierra};

		DrawSphere(tierra_posicion, 0.5f, BLUE);
		
		angulo_bola += 0.01f;

		const float epsilon = 0.000001f;

		if(angulo_bola > 2*PI + epsilon)
		{
			angulo_bola = 0; 

			not_full_circle = false;
		}

		if(not_full_circle)
			puntos_tierra.push_back(tierra_posicion);

		for(Vector3 vect: puntos_tierra)
			DrawPoint3D(vect, WHITE);

		//drawBezierCurve({distancia_tierra, 0, 0}, tierra_posicion, ballPosition, WHITE);

		DrawGrid(10, 1.0f); 
		
		EndMode3D();
		//Renderizar en pantalla
		rlImGuiEnd();
		EndDrawing();
    }

	rlImGuiShutdown();
    CloseWindow();

    return 0;
}