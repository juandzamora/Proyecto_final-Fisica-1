#include <cmath>
using std::sin;
using std::cos;
using std::acos;
using std::pow;
using std::sqrt;
#include <vector>
#include <string>
#include <iostream>

#include <raylib.h>
#include <raymath.h>

#include <rlImGui.h>
#include <rlImGuiColors.h>
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
	const float epsilon = 0.001f;

	if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
	{
		Vector2 mouse_delta = GetMouseDelta();

		const float sensibilidadX = 0.0075;
		const float sensibilidadY = 0.0025;

		angulo_x += mouse_delta.x * sensibilidadX;
		angulo_y += mouse_delta.y * sensibilidadY;
			
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
		float cambio = 1.5f;
		if(IsKeyDown(KEY_LEFT_SHIFT))
			cambio = 1.0f;
		if(IsKeyDown(KEY_RIGHT_CONTROL))
			cambio = 2.0f;

		float hipotenusa = sqrt(pow(camara.position.x - camara.target.x, 2) + pow(camara.position.y - camara.target.y, 2) + pow(camara.position.z - camara.target.z, 2)) - cambio;

		if(hipotenusa + epsilon > 0)
		{
			camara.position = {
				(cos(angulo_x)*sin(angulo_y)*hipotenusa) + camara.target.x, 
				(cos(angulo_y)*hipotenusa) + camara.target.y,
				(sin(angulo_x)*sin(angulo_y)*hipotenusa) + camara.target.z
			};
		}
	}

	if(IsKeyDown(KEY_S))
	{
		float cambio = 1.5f;
		if(IsKeyDown(KEY_LEFT_SHIFT))
			cambio = 1.0f;
		if(IsKeyDown(KEY_RIGHT_CONTROL))
			cambio = 2.0f;

		float hipotenusa = sqrt(pow(camara.position.x - camara.target.x, 2) + pow(camara.position.y - camara.target.y, 2) + pow(camara.position.z - camara.target.z, 2)) + cambio;

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

class cPlaneta
{
	private:
		Vector2 velocidad;
		Vector2 aceleracion;

		Vector2 posicion_inicial;
		Vector2 posicion;

		float tiempo_actual;

		std::string nombre_planeta;

		std::vector<Vector3> path;

		Color color;

		float radio;

		void actualizarVelocidad(double tiempo_que_paso)
		{
			velocidad.x = velocidad.x + aceleracion.x * (tiempo_que_paso/2.0);
			velocidad.y = velocidad.y + aceleracion.y * (tiempo_que_paso/2.0);
		}

		void actualizarAceleracion()
		{
			//Esto se veria como (reemplazar x o y dependiendo de la aceleración a calcular)
			/*
			* ax = aceleración en x
						
						        x
				ax = -1*-------------------
					     (x^2 + y^2)^(3/2)
			*/
			float denominador_aceleracion = pow(pow(posicion.x, 2) + pow(posicion.y, 2), 3.0 / 2.0);

			if(denominador_aceleracion == 0)
				denominador_aceleracion = 1;

			aceleracion.x = -1*(posicion.x/denominador_aceleracion);
			aceleracion.y = -1*(posicion.y/denominador_aceleracion);
		}


	public:
		cPlaneta(std::string nombre_planeta, Vector2 pos_inicial, float radio, Color color, Vector2 velocidad_inicial = {0, 0},bool ajustar_velocidad = true)
		{
			const float factor_de_escalado_radio = 1.0f/2400.0f;
			const float factor_de_escalado_distancia = 1.0f/2'500'000.0f;
			
			
			this->nombre_planeta = nombre_planeta;

			pos_inicial.x *= factor_de_escalado_distancia;

			if(ajustar_velocidad)
			{
				float v_orbital = sqrt(1.0f / pos_inicial.x);
				this->velocidad = {0, v_orbital * 0.5f};
			}
			else
				this->velocidad = velocidad_inicial;

			this->radio = radio * factor_de_escalado_radio;

			this->posicion_inicial = pos_inicial;

			this->posicion = pos_inicial;

			this->tiempo_actual = 0;

			this->color = color;
		}

		/*
		*/
		void updatePosition(double tiempo_que_paso)
		{
			tiempo_actual+= tiempo_que_paso;
			actualizarAceleracion();
			actualizarVelocidad(tiempo_que_paso);
			//Actualizar la posición
			posicion.x = posicion.x + velocidad.x * tiempo_que_paso;
			posicion.y = posicion.y + velocidad.y * tiempo_que_paso;
		}

		Vector2 getAceleracion()
		{
			return aceleracion;
		}

		Vector2 getVelocidad()
		{
			return velocidad;
		}

		float getRadio()
		{
			return radio;
		}

		Color getColor()
		{
			return color;
		}

		Vector3 getPosicion(float y_actual, bool esta_detenido)
		{
			Vector3 posicion_planeta = {posicion.x, y_actual,  posicion.y};
			
			if(path.size() < 10000 && !esta_detenido)
				path.push_back(posicion_planeta);

			return posicion_planeta;
		}

		std::vector<Vector3> getPath()
		{
			return path;
		}

		std::string getNombre()
		{
			return nombre_planeta;
		}
};




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

    Vector3 posicion_sol = { 0.0f, 0.0f, 0.0f};

	const int max_fps = 144;

    SetTargetFPS(max_fps);
	
	//rlImGui setup
	rlImGuiSetup(true); 
	ImGuiIO& io = ImGui::GetIO();

	int velocidad = 1;

	bool boton_abajo = false;

	float angulo_y = 0.0f;
	float angulo_x = 0.0f;

	{
		float hipotenusa = sqrt(pow(camara.position.x - camara.target.x, 2) + pow(camara.position.y - camara.target.y, 2) + pow(camara.position.z - camara.target.z, 2));
		angulo_y = acos(camara.position.y/hipotenusa);
		angulo_x = acos(camara.position.x/(sin(angulo_y) * hipotenusa));
	}

	//Datos de https://planetario.buenosaires.gob.ar/sites/default/files/2018-09/Tablas-%20El%20sistema%20solar%20en%20numeros-docentes.pdf y https://science.nasa.gov/resource/solar-system-sizes/. Todos los datos estan sus km reales, excepto el sol
	std::vector<cPlaneta> lista_planetas = {
		cPlaneta("Sol", {0, 0}, 15000.0f, ORANGE, {0, 0}, false),
		cPlaneta("Mercurio", {57909175.0f, 0}, 2440, LIGHTGRAY),
		cPlaneta("Venus", {108208930.0f, 0}, 6052, BEIGE),			
		cPlaneta("Tierra", {149597890.0f, 0}, 6371, BLUE),
		cPlaneta("Marte", {227936640.0f , 0}, 3390, RED),
		cPlaneta("Jupiter", {778412020.0f, 0}, 69911, ORANGE),
		cPlaneta("Saturno", {1426725400.0f, 0}, 58232, GOLD),
		cPlaneta("Urano", { 2870972200.0f, 0}, 25362, SKYBLUE),
		cPlaneta("Neptuno", {4498252900.0f, 0}, 24622, DARKBLUE)
	};

	float delta_tiempo = 0.1;
	bool pausar_simulacion = false;



    while (!WindowShouldClose())
	{	
		BeginDrawing();
		rlImGuiBegin();

		ClearBackground(BLACK); 
		BeginMode3D(camara);

		//Mover la pelota
		ImGui::Begin("Menu");
		ImGui::TextWrapped("Configuracion general");
		ImGui::DragInt("Velocidad", &velocidad, 1, 0);
		
		static std::string texto_pausa;

		if(pausar_simulacion)
			texto_pausa = "Despausar simulacion";
		else
			texto_pausa = "Pausar simulacion";

		if(ImGui::Button(texto_pausa.c_str()))
			pausar_simulacion = !pausar_simulacion;
		//TODO
		if(!ImGui::Button("Cargar de archivo (proximamente)"));

		//Lista planetas
		if(ImGui::CollapsingHeader("Ver lista de planetas"))
		{
			ImGui::Indent();
			for(cPlaneta & planeta : lista_planetas)
			{
				if (!ImGui::CollapsingHeader(planeta.getNombre().c_str()))
					continue;

				Vector3 posicion = planeta.getPosicion(posicion_sol.y, pausar_simulacion);
				Vector2 velocidad = planeta.getVelocidad();
				Vector2 aceleracion = planeta.getAceleracion();
				ImGui::Indent();
					ImGui::Text("Posicion del planeta: ");
					ImGui::SameLine();
					ImGui::TextColored(rlImGuiColors::Convert(BLUE), " x: %.4f, y: %.4f", posicion.x, posicion.z);

					ImGui::Text("Velocidad del planeta: ");
					ImGui::SameLine();
					ImGui::TextColored(rlImGuiColors::Convert(BLUE), " x: %.4f, y: %.4f", velocidad.x, velocidad.y);
				
					ImGui::Text("Aceleracion del planeta: ");
					ImGui::SameLine();
					ImGui::TextColored(rlImGuiColors::Convert(BLUE), " x: %.4f, y: %.4f", aceleracion.x, aceleracion.y);	
				ImGui::Unindent();
			}
			ImGui::Unindent();
		}
		
		ImGui::End();
		

		if(!pausar_simulacion)
		{
			for(int i = 0; i < velocidad; i++)
				for(cPlaneta & planeta : lista_planetas)
					planeta.updatePosition(delta_tiempo);
		}

		//DrawSphere(tierra.getPosicion(ballPosition.y),0.5f,BLUE);
		for(cPlaneta & planeta : lista_planetas)
			DrawSphere(planeta.getPosicion(posicion_sol.y, pausar_simulacion), planeta.getRadio(), planeta.getColor());
		
		for(cPlaneta & planeta : lista_planetas)
			for(Vector3 & vect: planeta.getPath())
				DrawPoint3D(vect, WHITE);

		//DrawGrid(100, 40.0f); 

		EndMode3D();
		//Renderizar en pantalla
		rlImGuiEnd();
		EndDrawing();

		//input
		if(!io.WantCaptureKeyboard)
			manageInput(camara, angulo_x, angulo_y);
    }

	rlImGuiShutdown();
    CloseWindow();

    return 0;
}