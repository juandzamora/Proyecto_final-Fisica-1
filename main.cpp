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

#include <implot.h>

#include "camara.h"
#include "planeta.h"


int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Proyecto final - Fisica 1");
	MaximizeWindow();

	cCamara3DCustom camara({40.0f, 40.0f, 40.0f}, {0.0f, 0.0f, 0.0f});
	
	const int max_fps = 144;

    SetTargetFPS(max_fps);
	
	//rlImGui setup
	rlImGuiSetup(true); 
	ImGuiIO& io = ImGui::GetIO();
	ImPlot::CreateContext();

	//Datos de https://planetario.buenosaires.gob.ar/sites/default/files/2018-09/Tablas-%20El%20sistema%20solar%20en%20numeros-docentes.pdf, https://science.nasa.gov/resource/solar-system-sizes/. 
	// https://spaceplace.nasa.gov/planets-weight/sp/ y Todos los datos estan en sus km reales, excepto el sol
	std::vector<cPlaneta> lista_planetas = {
		cPlaneta("Sol", 0, 0, 0, 15000.0f, ORANGE, {0, 0}, false, true),
		cPlaneta("Mercurio", 57909175.0f,  0.0553f, 88, 2440, LIGHTGRAY),
		cPlaneta("Venus", 108208930.0f, 0.815f, 225, 6052, BEIGE),			
		cPlaneta("Tierra", 149597890.0f, 1.0f, 365, 6371, BLUE),
		cPlaneta("Marte", 227936640.0f, 0.107f, 687, 3390, RED),
		cPlaneta("Jupiter", 778412020.0f, 318.0f, 4333, 69911, ORANGE),
		cPlaneta("Saturno", 1426725400.0f, 95.2f, 10759, 58232, GOLD),
		cPlaneta("Urano", 2870972200.0f, 14.5f, 30687, 25362, SKYBLUE),
		cPlaneta("Neptuno", 4498252900.0f, 17.1f, 60190, 24622, DARKBLUE)
	};

	//Variables de la simulación
	const float delta_tiempo = 0.1;
	int velocidad = 1;

	//Size t que recive negativos (no es como que size_t no lo haga, pero aja, por si acaso. size_t es unsigned)
	long long index_vista_fija = -1;
	bool fijar_planeta = false;


	int dia_a_parar;

	float tiempo_funcionando = 0.0f;
	bool plotear_toda_velocidad = false;
	bool plotear_toda_energia_cinetica = false;
	bool plotear_toda_aceleracion = false;
    while (!WindowShouldClose())
	{	
		BeginDrawing();
		rlImGuiBegin();

		ClearBackground(BLACK); 
		BeginMode3D(camara.getCamara());


		ImGui::Begin("Menu");
		ImGui::TextWrapped("Configuracion general");
		ImGui::DragInt("Velocidad", &velocidad, 1, 0, 1000);
		
		std::string texto_pausa;
		static int velocidad_antes_de_pausa;
		if(velocidad <= 0)
			texto_pausa = "Despausar simulacion";
		else
			texto_pausa = "Pausar simulacion";

		
		if(ImGui::Button(texto_pausa.c_str()))
		{
			if(velocidad <= 0)
				velocidad = 1;
			else
				velocidad = 0;
		}

		ImGui::TextWrapped("Dia actual: %.2f", (365.0f/2238.73f) * tiempo_funcionando);

		ImGui::InputInt("Dia a parar", &dia_a_parar, 0);

		if(dia_a_parar <= (365.0f/2238.73f) * tiempo_funcionando && dia_a_parar > 0)
		{
			velocidad = 0;
			dia_a_parar = 0;
		}

		std::string texto_plot_velocidad;
		
		if(plotear_toda_velocidad)
			texto_plot_velocidad = "Pausar plot velocidad";
		else
			texto_plot_velocidad = "Plotear toda la velocidad";

		if(ImGui::Button(texto_plot_velocidad.c_str()))
		{
			plotear_toda_velocidad = !plotear_toda_velocidad;

			for(cPlaneta & planeta : lista_planetas)
			{
				if(planeta.getSol()) 
					continue;

				if(plotear_toda_velocidad)
				{
					if(planeta.getEstadoGrafica(dat_velocidad) == pausada)
						planeta.cambiarEstadoRecoleccionDatos(dat_velocidad, pausada);
					else
						planeta.cambiarEstadoRecoleccionDatos(dat_velocidad, activa);
				}
				else
					planeta.cambiarEstadoRecoleccionDatos(dat_velocidad, pausada);
				
			}
		}

		std::string texto_recoger_data_velocidad = "Recoger velocidad de todos";
		ImGui::SameLine();

		if(ImGui::Button(texto_recoger_data_velocidad.c_str()))
		{

			for(cPlaneta & planeta : lista_planetas)
			{
				if(planeta.getSol()) 
					continue;

				planeta.guardarGrafica(dat_velocidad);
			}
		}

		std::string texto_plot_aceleracion;
		
		if(plotear_toda_aceleracion)
			texto_plot_aceleracion = "Pausar plot aceleracion";
		else
			texto_plot_aceleracion = "Plotear toda la aceleracion";

		if(ImGui::Button(texto_plot_aceleracion.c_str()))
		{
			plotear_toda_aceleracion = !plotear_toda_aceleracion;

			for(cPlaneta & planeta : lista_planetas)
			{
				if(planeta.getSol()) 
					continue;

				if(plotear_toda_aceleracion)
				{
					if(planeta.getEstadoGrafica(dat_aceleracion) == pausada)
						planeta.cambiarEstadoRecoleccionDatos(dat_aceleracion, pausada);
					else
						planeta.cambiarEstadoRecoleccionDatos(dat_aceleracion, activa);
				}
				else
					planeta.cambiarEstadoRecoleccionDatos(dat_aceleracion, pausada);
				
			}
		}

		std::string texto_recoger_data_aceleracion = "Recoger aceleracion de todos";
		ImGui::SameLine();

		if(ImGui::Button(texto_recoger_data_aceleracion.c_str()))
		{

			for(cPlaneta & planeta : lista_planetas)
			{
				if(planeta.getSol()) 
					continue;

				planeta.guardarGrafica(dat_aceleracion);
			}
		}

		std::string texto_plot_energia_cinetica;
		
		if(plotear_toda_energia_cinetica)
			texto_plot_energia_cinetica = "Pausar plot energia cinetica";
		else
			texto_plot_energia_cinetica = "Plotear toda la energia cinetica";

		if(ImGui::Button(texto_plot_energia_cinetica.c_str()))
		{
			plotear_toda_energia_cinetica = !plotear_toda_energia_cinetica;

			for(cPlaneta & planeta : lista_planetas)
			{
				if(planeta.getSol()) 
					continue;

				if(plotear_toda_energia_cinetica)
				{
					if(planeta.getEstadoGrafica(dat_energia_cinetica) == pausada)
						planeta.cambiarEstadoRecoleccionDatos(dat_energia_cinetica, pausada);
					else
						planeta.cambiarEstadoRecoleccionDatos(dat_energia_cinetica, activa);
				}
				else
					planeta.cambiarEstadoRecoleccionDatos(dat_energia_cinetica, pausada);
				
			}
		}

		std::string texto_recoger_data_energia_cinetica = "Recoger energia cinetica de todos";
		
		ImGui::SameLine();

		if(ImGui::Button(texto_recoger_data_energia_cinetica.c_str()))
		{

			for(cPlaneta & planeta : lista_planetas)
			{
				if(planeta.getSol()) 
					continue;

				planeta.guardarGrafica(dat_energia_cinetica);
			}
		}

		//Lista planetas
		if(ImGui::CollapsingHeader("Ver lista de planetas"))
		{
			long long index_actual = -1;
			for(cPlaneta & planeta : lista_planetas)
			{
				ImGui::Indent();
					index_actual++;
					if(!ImGui::CollapsingHeader(planeta.getNombre().c_str()))
					{
						ImGui::Unindent();
						continue;
					}

					Vector3 posicion = planeta.getPosicion();
					Vector2 velocidad = planeta.getVelocidad();
					Vector2 aceleracion = planeta.getAceleracion();
					ImGui::Indent();

						ImGui::Text("Posicion del planeta: ");
						ImGui::SameLine();
						ImGui::TextColored(rlImGuiColors::Convert(BLUE), " x: %.4f km, y: %.4f km", posicion.x, posicion.z);

						ImGui::Text("Velocidad del planeta: ");
						ImGui::SameLine();
						ImGui::TextColored(rlImGuiColors::Convert(BLUE), " x: %.4f km/s, y: %.4f km/s", velocidad.x, velocidad.y);
	
						ImGui::Text("Aceleracion del planeta: ");
						ImGui::SameLine();
						ImGui::TextColored(rlImGuiColors::Convert(BLUE), " x: %.6f km/(s^2), y: %.6f km/(s^2)", aceleracion.x, aceleracion.y);

						std::string fijar_camara_texto = "Apuntar camara al planeta##Apuntar_camara_" + std::to_string(index_actual);

						if(index_vista_fija == index_actual)
						{
							ImGui::BeginDisabled(true);
							ImGui::Button(fijar_camara_texto.c_str());
							ImGui::EndDisabled();
							ImGui::SameLine();
							ImGui::Checkbox("Fijarse a planeta", &fijar_planeta);
						}
						else if(ImGui::Button(fijar_camara_texto.c_str()))
							index_vista_fija = index_actual;


						std::string registrar_datos = "Registrar datos##Registar_datos_" + std::to_string(index_actual);
						
						if(planeta.getSol())
						{
							ImGui::Unindent();
							ImGui::Unindent();
							continue;
						}

						if(!ImGui::CollapsingHeader(registrar_datos.c_str()))
						{	
							ImGui::Unindent();
							ImGui::Unindent();
							continue;
						}
						ImGui::Indent();
						//Botones de ayuda para graficas
						{
							if(ImGui::Button(std::string("Iniciar grafica velocidad##iniciar_datos_velocidad_" + std::to_string(index_actual)).c_str()))
								planeta.cambiarEstadoRecoleccionDatos(dat_velocidad, activa);
							ImGui::SameLine();
							if(ImGui::Button(std::string("Pausar grafica velocidad##pausa_datos_velocidad_" + std::to_string(index_actual)).c_str()))
								planeta.cambiarEstadoRecoleccionDatos(dat_velocidad, pausada);
							ImGui::SameLine();
							if(ImGui::Button(std::string("Borrar grafica velocidad##eliminar_datos_velocidad_" + std::to_string(index_actual)).c_str()))
								planeta.cambiarEstadoRecoleccionDatos(dat_velocidad, vacia);

							planeta.verGraficoVelocidad();
							
							if(ImGui::Button(std::string("Guardar datos de velocidad##guardar_datos_velocidad_" + std::to_string(index_actual)).c_str()))
								planeta.guardarGrafica(dat_velocidad);
							
							if(ImGui::Button(std::string("Iniciar grafica aceleracion##iniciar_datos_aceleracion_" + std::to_string(index_actual)).c_str()))
								planeta.cambiarEstadoRecoleccionDatos(dat_aceleracion, activa);
							ImGui::SameLine();
							if(ImGui::Button(std::string("Pausar grafica aceleracion##pausa_datos_aceleracion_" + std::to_string(index_actual)).c_str()))
								planeta.cambiarEstadoRecoleccionDatos(dat_aceleracion, pausada);
							ImGui::SameLine();
							if(ImGui::Button(std::string("Borrar grafica aceleracion##eliminar_datos_aceleracion_" + std::to_string(index_actual)).c_str()))
								planeta.cambiarEstadoRecoleccionDatos(dat_aceleracion, vacia);

							planeta.verGraficoAceleracion();

							if(ImGui::Button(std::string("Guardar datos de aceleracion##guardar_datos_aceleracion_" + std::to_string(index_actual)).c_str()))
								planeta.guardarGrafica(dat_aceleracion);

							if(ImGui::Button(std::string("Iniciar grafica energia cinetica##iniciar_datos_energia_cinetica_" + std::to_string(index_actual)).c_str()))
								planeta.cambiarEstadoRecoleccionDatos(dat_energia_cinetica, activa);
							ImGui::SameLine();
							if(ImGui::Button(std::string("Pausar grafica energia cinetica##pausa_datos_energia_cinetica_" + std::to_string(index_actual)).c_str()))
								planeta.cambiarEstadoRecoleccionDatos(dat_energia_cinetica, pausada);
							ImGui::SameLine();
							if(ImGui::Button(std::string("Borrar grafica energia cinetica##eliminar_datos_energia_cinetica_" + std::to_string(index_actual)).c_str()))
								planeta.cambiarEstadoRecoleccionDatos(dat_energia_cinetica, vacia);

							planeta.verGraficoEnergiaCinetica();

							if(ImGui::Button(std::string("Guardar datos de energia cinetica##guardar_datos_energia_cinetica_" + std::to_string(index_actual)).c_str()))
								planeta.guardarGrafica(dat_energia_cinetica);
						}
						ImGui::Unindent();	

					ImGui::Unindent();
				ImGui::Unindent();
			}
		}		
		ImGui::End();

		if(index_vista_fija != -1)
			camara.cambiarPlanetaTarget(lista_planetas[index_vista_fija], index_vista_fija, fijar_planeta);

		for(int i = 0; i < velocidad; i++)
			for(cPlaneta & planeta : lista_planetas)
			{
				float tiempo_vuelta_anterior = planeta.getTiempoVueltaAnterior();
					
				//if(planeta.completoUnPeriodo())
					//std::cout << "\nPeriodo completo de " + planeta.getNombre() + " en " << planeta.getTiempoVueltaAnterior() - tiempo_vuelta_anterior;
				tiempo_funcionando += delta_tiempo;
				planeta.updatePosition(delta_tiempo);
			}

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
	ImPlot::DestroyContext();
	rlImGuiShutdown();
    CloseWindow();

    return 0;
}