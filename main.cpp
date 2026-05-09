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

static float z_de_todo_el_proyecto = 0.0f;

class cPlaneta
{
	private:
		//Caracteristicas del plantea
		std::string nombre_planeta;

		Vector2 velocidad;
		Vector2 aceleracion;

		Vector2 posicion;
		//Para hacer que la camara se mueva con el planeta
		Vector2 posicion_anterior;

		float tiempo_orbitando;

		//Caracteristicas del planeta para renderizarlo
		Color color;
		float radio;

		//Variables de ayuda para Path de aqui para abajo:
		std::vector<Vector3> path;
		const float intervalo_de_guardado = 2.0f;
		//Para guardar un intervalo para saber cuando se llegue a intervalo_de_guardado
		float intervalo_tiempo;

		bool es_sol;

		//Para reescribir path cuando se haya completado una vuelta
		size_t index_actual;

		//Guardado del angulo previo 
		float angulo_anterior;

		//Identifican si ya completo su primera vuelta y si ya dio la primera media vuelta desde su origen (esta ultima se reinicia cada vez que pasa aprox por el origen)
		bool completo_una_vuelta;
		bool dio_media_vuelta;

		void modificarPath(float dt)
		{
			if(es_sol)
				return;

			this->intervalo_tiempo += dt;

			if(intervalo_tiempo >= intervalo_de_guardado)
			{
				float angulo = atan2(posicion.y, posicion.x);

				this->intervalo_tiempo = 0.0f;

				if(angulo > 0 && angulo_anterior < 0)
				{
					completo_una_vuelta = true;
					dio_media_vuelta = false;

					index_actual = 0;
				}

				if(completo_una_vuelta)
				{
					if(index_actual < path.size())
					{
						path[index_actual] = getPosicion();

						index_actual++;
					}
				}
				else
					path.push_back(getPosicion());

				//Dio una media vuelta
				if(angulo < 0 && angulo_anterior > 0)
				{
					dio_media_vuelta = true;
				}

				angulo_anterior = angulo;
			}
		}

		void actualizarVelocidad(double dt)
		{
			velocidad.x = velocidad.x + aceleracion.x * (dt/2.0);
			velocidad.y = velocidad.y + aceleracion.y * (dt/2.0);
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
		cPlaneta(std::string nombre_planeta, Vector2 pos_inicial, float radio, Color color, Vector2 velocidad_inicial = {0, 0},bool ajustar_velocidad = true, bool es_sol = false)
		{
			const float factor_de_escalado_radio = 1.0f/2400.0f;
			const float factor_de_escalado_distancia = 1.0f/2'500'000.0f;

			this->nombre_planeta = nombre_planeta;

			//Tiempo que a "pasado" (o simulado) desde que empezo la simulación. Se actualiza con dt
			this->tiempo_orbitando = 0;


			//inicializacion variables necesarias para Path
			{
				//Para saber si es el sol (y no dibujarle path)
				this->es_sol = es_sol;

				this->completo_una_vuelta = false;
				this->dio_media_vuelta = false;

				this->angulo_anterior = 0;

				//Para que esto funcione, dt tiene que ser constante
				this->intervalo_tiempo = 0;
			}

			pos_inicial.x *= factor_de_escalado_distancia;

			if(ajustar_velocidad)
			{
				float v_orbital = sqrt(1.0f / pos_inicial.x);
				this->velocidad = {0, v_orbital * 0.5f};
			}
			else
				this->velocidad = velocidad_inicial;

			this->radio = radio * factor_de_escalado_radio;

			this->posicion = pos_inicial;

			this->color = color;
		}

		//dt = delta de tiempo, tiempo que paso entre el anterior calculo (o llamado a esta función) a este. Es simulado, no tiene que ser realista.
		void updatePosition(double dt)
		{
			tiempo_orbitando += dt;
			actualizarAceleracion();
			actualizarVelocidad(dt);
			//Actualizar la posición
			posicion.x = posicion.x + velocidad.x * dt;
			posicion.y = posicion.y + velocidad.y * dt;

			modificarPath(dt);
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

		Vector3 getPosicion()
		{
			return {posicion.x, z_de_todo_el_proyecto,  posicion.y};
		}

		Vector3 getPosicionAnterior()
		{
			return {posicion_anterior.x, z_de_todo_el_proyecto,  posicion_anterior.y};
		}

		std::vector<Vector3> & getPath()
		{
			return path;
		}

		std::string getNombre()
		{
			return nombre_planeta;
		}

		bool yaDioUnaVuelta()
		{
			return completo_una_vuelta;
		}

		size_t getTamañoPath()
		{
			return path.size();
		}
};

class cCamara3DCustom
{
	private:
		Camera3D camara;

		//Los angulo en X e Y de la camara
		float angulo_y;
		float angulo_x;

		const float epsilon = 0.01f;

		bool mantener_fija_en_planeta;
		float factor_zoom_cuando_fijo;

		inline float getHipotenusa()
		{
			return sqrt(pow(camara.position.x - camara.target.x, 2) + pow(camara.position.y - camara.target.y, 2) + pow(camara.position.z - camara.target.z, 2));
		}

		inline void actualizarPosicion(const float & hipotenusa)
		{	
			camara.position = {
				(cos(angulo_x)*sin(angulo_y)*hipotenusa) + camara.target.x, 
				(cos(angulo_y)*hipotenusa) + camara.target.y,
				(sin(angulo_x)*sin(angulo_y)*hipotenusa) + camara.target.z
			};
		}

		void modificarZoom(float factor_de_zoom)
		{
			float hipotenusa = getHipotenusa() + factor_de_zoom;

			if(hipotenusa + epsilon > 0)
				actualizarPosicion(hipotenusa);
		}

		void moverseEnElEje()
		{
			Vector2 mouse_delta = GetMouseDelta();

			const float sensibilidadX = 0.0075;
			const float sensibilidadY = 0.0025;

			angulo_x += mouse_delta.x * sensibilidadX;
			angulo_y += mouse_delta.y * sensibilidadY;
			
			angulo_y = Clamp(angulo_y, 0 + epsilon, PI - epsilon);

			actualizarPosicion(getHipotenusa());
		}

		void fijarFirmeAPlaneta(Vector3 posicion_planeta, Vector3 posicion_fija)
		{
			camara.target = posicion_planeta;

			camara.position = posicion_fija;

			float hipotenusa = sqrt(pow(posicion_fija.x, 2) + pow(posicion_fija.y, 2) + pow(posicion_fija.z, 2)) + this->factor_zoom_cuando_fijo;

			angulo_y = acos((posicion_fija.y - camara.target.y)/hipotenusa);
			angulo_x = atan2(posicion_fija.z - camara.target.z, posicion_fija.x - camara.target.x);

			if(hipotenusa + epsilon > 0)
				actualizarPosicion(hipotenusa);
		}

	public:
		
		cCamara3DCustom(Vector3 posicion_inicial, Vector3 target_inicial, float fovy = 45.0f)
		{
			camara = {0};
			
			camara.position = posicion_inicial;
			camara.target = target_inicial;

			camara.fovy = fovy;
			camara.up = { 0.0f, 1.0f, 0.0f }; 
			camara.projection = CAMERA_PERSPECTIVE;

			float hipotenusa = getHipotenusa();

			angulo_y = acos(camara.position.y/hipotenusa);
			angulo_x = atan2(camara.position.z - camara.target.z, camara.position.x - camara.target.x);

			this->factor_zoom_cuando_fijo = 0.0f;
			this->mantener_fija_en_planeta = false;
		}

		void detectInput()
		{
			if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
				moverseEnElEje();

			float cambio = 1.5f;
			if(IsKeyDown(KEY_LEFT_SHIFT))
				cambio = 1.0f;
			if(IsKeyDown(KEY_LEFT_CONTROL))
				cambio = 2.0f;

			if(mantener_fija_en_planeta)
			{
				if(IsKeyDown(KEY_W))
					this->factor_zoom_cuando_fijo += cambio * -1;

				if(IsKeyDown(KEY_S))
					this->factor_zoom_cuando_fijo += cambio;
				
				return;
			}

			if(IsKeyDown(KEY_W))
				modificarZoom(-1*cambio);

			if(IsKeyDown(KEY_S))
				modificarZoom(cambio);
		}

		Camera3D & getCamara()
		{
			return camara;
		}
		
		void cambiarPlanetaTarget(cPlaneta & planeta, bool mantener_fija_en_planeta, Vector3 posicion_fija = {0.0f, 40.0f, 40.0f})
		{
			if(mantener_fija_en_planeta)
			{
				fijarFirmeAPlaneta(planeta.getPosicion(), posicion_fija);
				this->mantener_fija_en_planeta = true;

				return;
			}

			this->mantener_fija_en_planeta = false;
			this->factor_zoom_cuando_fijo = 0.0f;
			camara.target = planeta.getPosicion();

			float hipotenusa = getHipotenusa();

			angulo_y = acos(camara.position.y/hipotenusa);
			angulo_x = atan2(camara.position.z - camara.target.z, camara.position.x - camara.target.x);

			actualizarPosicion(hipotenusa);
		}
};

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Proyecto final - Fisica 1");

	cCamara3DCustom camara({0.0f, 40.0f, 40.0f}, {0.0f, 0.0f, 0.0f});

	const int max_fps = 144;

    SetTargetFPS(max_fps);
	
	//rlImGui setup
	rlImGuiSetup(true); 
	ImGuiIO& io = ImGui::GetIO();
	
	//Datos de https://planetario.buenosaires.gob.ar/sites/default/files/2018-09/Tablas-%20El%20sistema%20solar%20en%20numeros-docentes.pdf y https://science.nasa.gov/resource/solar-system-sizes/. Todos los datos estan en sus km reales, excepto el sol
	std::vector<cPlaneta> lista_planetas = {
		cPlaneta("Sol", {0, 0}, 15000.0f, ORANGE, {0, 0}, false, true),
		cPlaneta("Mercurio", {57909175.0f, 0}, 2440, LIGHTGRAY),
		cPlaneta("Venus", {108208930.0f, 0}, 6052, BEIGE),			
		cPlaneta("Tierra", {149597890.0f, 0}, 6371, BLUE),
		cPlaneta("Marte", {227936640.0f , 0}, 3390, RED),
		cPlaneta("Jupiter", {778412020.0f, 0}, 69911, ORANGE),
		cPlaneta("Saturno", {1426725400.0f, 0}, 58232, GOLD),
		cPlaneta("Urano", { 2870972200.0f, 0}, 25362, SKYBLUE),
		cPlaneta("Neptuno", {4498252900.0f, 0}, 24622, DARKBLUE)
	};

	//Variables de la simulación
	const float delta_tiempo = 0.1;
	bool pausar_simulacion = false;
	int velocidad = 1;

	//Size t que recive negativos (no es como que size_t no lo haga, pero aja, por si acaso. size_t es unsigned)
	long long index_vista_fija = -1;
	bool fijar_planeta = false;
	Vector3 posicion_fija_camara = {0.0f, 40.0f, 40.0f};
    while (!WindowShouldClose())
	{	
		BeginDrawing();
		rlImGuiBegin();

		ClearBackground(BLACK); 
		BeginMode3D(camara.getCamara());


		ImGui::Begin("Menu");
		ImGui::TextWrapped("Configuracion general");
		ImGui::DragInt("Velocidad", &velocidad, 1, 0, 1000);
		ImGui::SameLine();
		ImGui::InputInt(" ", &velocidad);
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
			long long index_actual = -1;
			ImGui::Indent();
			for(cPlaneta & planeta : lista_planetas)
			{
				index_actual++;
				if (!ImGui::CollapsingHeader(planeta.getNombre().c_str()))
					continue;

				Vector3 posicion = planeta.getPosicion();
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


					std::string fijar_camara_texto = "Apuntar camara al planeta##" + std::to_string(index_actual);

					if(index_vista_fija == index_actual)
					{
						ImGui::BeginDisabled(true);
						ImGui::Button(fijar_camara_texto.c_str());
						ImGui::EndDisabled();
						ImGui::SameLine();
						ImGui::Checkbox("Fijarse a planeta", &fijar_planeta);
							ImGui::DragFloat("x", &posicion_fija_camara.x, 0.1f);
							ImGui::DragFloat("y", &posicion_fija_camara.y, 0.1f);
							ImGui::DragFloat("z", &posicion_fija_camara.z, 0.1f);
					}
					else if(ImGui::Button(fijar_camara_texto.c_str()))
						index_vista_fija = index_actual;

				ImGui::Unindent();
			}
			ImGui::Unindent();
		}		
		ImGui::End();
		
		if(index_vista_fija != -1)
			camara.cambiarPlanetaTarget(lista_planetas[index_vista_fija], fijar_planeta, posicion_fija_camara);

		if(!pausar_simulacion)
		{
			for(int i = 0; i < velocidad; i++)
				for(cPlaneta & planeta : lista_planetas)
					planeta.updatePosition(delta_tiempo);
		}

		//DrawSphere(tierra.getPosicion(ballPosition.y),0.5f,BLUE);
		for(cPlaneta & planeta : lista_planetas)
			DrawSphere(planeta.getPosicion(), planeta.getRadio(), planeta.getColor());
		

		for(cPlaneta & planeta : lista_planetas)
		{
			std::vector<Vector3>& puntos = planeta.getPath();

			for(size_t index = 1; index < puntos.size(); index++)
			{
				if(index + 1 == puntos.size() && planeta.yaDioUnaVuelta())
					DrawLine3D(puntos[index], puntos[0], WHITE);

				DrawLine3D(puntos[index - 1], puntos[index], WHITE);
			}
		}

		//DrawGrid(100, 40.0f); 

		EndMode3D();
		//Renderizar en pantalla
		rlImGuiEnd();
		EndDrawing();

		//input
		if(!io.WantCaptureKeyboard)
			camara.detectInput();
    }

	rlImGuiShutdown();
    CloseWindow();

    return 0;
}