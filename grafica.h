#pragma once
#include <raylib.h>
#include <raymath.h>

#include <vector>
#include <string>
#include <iostream>
#include <float.h>
#include <implot.h>
#include <fstream>

enum eComponentesGrafica
{
	titulo,
	x,
	y
};

enum eEstadoGrafica
{
	activa,
	pausada,
	vacia
};

struct sVector2Int
{
	int x, y;
};

class cGrafica 
{
	private:
		std::vector<Vector2> lista_data;
		std::vector<int> series;

		std::string nombre_grafica;
		
		std::string nombre_x;
		std::string nombre_y;
		
		//Constante a la cual se necesita comparar.
		std::string nombre_valor_inicial;
		float valor_inicial;
		
		//Datos para la grafica.
		unsigned int duracion_de_año_planeta;
		float minimo_y;
		float maximo_y;
	public:

		cGrafica(){ }
		cGrafica(std::string nombre_grafica, std::string nombre_x, std::string nombre_y, std::string nombre_valor_inicial,float valor_inicial, unsigned int duracion_de_año_planeta)
			:nombre_grafica(nombre_grafica), nombre_x(nombre_x), nombre_y(nombre_y), nombre_valor_inicial(nombre_valor_inicial), 
			 valor_inicial(valor_inicial), duracion_de_año_planeta(duracion_de_año_planeta), minimo_y(valor_inicial - 1), maximo_y(valor_inicial + 1)
		{
			this->series.push_back(0);
		}

		void añadirData(Vector2 data)
		{
			lista_data.push_back(data);
			if(data.y > maximo_y)
				maximo_y = data.y;
			
			if(data.y < minimo_y)
				minimo_y = data.y;
		}

		void limpiarData()
		{
			lista_data.clear();
			series.clear();
			maximo_y = valor_inicial + 1;
			minimo_y = valor_inicial - 1; 
		}
		//ImPlotSpec
		std::vector<Vector2> obtenerData()
		{
			return lista_data;
		}

		void cambiarNombre(std::string nombre,eComponentesGrafica componente)
		{
			if(componente == titulo)
				this->nombre_grafica = nombre;
			if(componente == x)
				this->nombre_x = nombre;
			if(componente == y)
				this->nombre_y = nombre;
		}

		void terminarSerie()
		{
			lista_data.push_back({NAN, NAN});
			series.push_back(this->lista_data.size());
		}


		//Esto obligatoriamente necesita ser llamado adentro de un recudro de Dear ImGui
		void visualizarGrafica()
		{	
			if(lista_data.size() < 3) 
				return;

			if(ImPlot::BeginPlot(std::string(nombre_grafica).c_str()))
			{
				ImPlot::SetupAxis(ImAxis_X1, nombre_x.c_str());
				ImPlot::SetupAxis(ImAxis_Y1, nombre_y.c_str());
				
				float dia_inicio_grafica = lista_data.front().x;
				float dia_final_grafica = lista_data.back().x;

				if(std::isnan(dia_final_grafica))
					dia_final_grafica = lista_data[lista_data.size() - 2].x;

				ImPlot::SetupAxisLimits(ImAxis_X1, dia_inicio_grafica, dia_final_grafica + 10, ImGuiCond_Always);
				ImPlot::SetupAxisLimits(ImAxis_Y1, maximo_y, minimo_y, ImGuiCond_Once);
				//Dibujar la grafica
				ImPlot::PlotLine(nombre_grafica.c_str(), &lista_data.front().x,&lista_data.front().y,lista_data.size(),{ImPlotProp_Stride, sizeof(Vector2)});

				//Dibujar el valor de y en el primer momento de la simulación
				ImPlot::PlotInfLines(std::string(this->nombre_valor_inicial + "##Valor_inicial" + this->nombre_grafica).c_str(), &this->valor_inicial, 1, {ImPlotProp_Flags, ImPlotInfLinesFlags_Horizontal});

				//Para dibujar las linéas cuando se completa el equivalente a un año en ese planeta
				std::vector<float> dias_donde_pasan_los_años;
				for(int i = dia_inicio_grafica/duracion_de_año_planeta; i < dia_final_grafica/duracion_de_año_planeta; i++)
					dias_donde_pasan_los_años.push_back(duracion_de_año_planeta*i);

				if (!dias_donde_pasan_los_años.empty())
					ImPlot::PlotInfLines(std::string("Vuelta completa alrededor del sol##Lineas_horizontal_año-"+nombre_grafica).c_str(), 
										 &dias_donde_pasan_los_años.front(), dias_donde_pasan_los_años.size());

				ImPlot::EndPlot();
			}
		}

		void guardarInformacion()
		{
			std::ofstream archivo_informacion(this->nombre_grafica + ".csv");

			if(!archivo_informacion.is_open())
			{
				std::cerr << "No se pudo abrir el archivo: " << this->nombre_grafica << ".csv" << std::endl;
				return; // salir limpiamente sin dejar nada colgado
			}

			std::vector<sVector2Int> rangos_reales;

			int rango_max = 0;

			for(int i = 0; i < series.size(); i++)
			{
				if(i + 1 == series.size())
					break;

				rangos_reales.push_back({series[i], series[i + 1] - series[i] - 1});

				if(rangos_reales.back().y > rango_max)
					rango_max = rangos_reales.back().y;
			}

			std::string primera_linea = "Datos principales,";
			for(int i = 0; i < rangos_reales.size(); i++)
				primera_linea += "Tiempo serie " + std::to_string(i + 1) + "," + "Datos serie " + std::to_string(i + 1) + ",";

			archivo_informacion << primera_linea + "\n";

			for(int i = 0; i < rango_max; i++)
			{
				std::string linea;
				if(i == 0)
					linea = this->nombre_grafica;
				if(i == 1)
					linea = std::to_string(this->duracion_de_año_planeta);
				if(i == 2)
					linea = std::to_string(this->valor_inicial);

				linea += ",";

				for(const auto & rango : rangos_reales)
				{
					if(rango.y <= i)
					{
						linea += ",,";
						continue;
					}

					linea += std::to_string(lista_data[rango.x + i].x) + "," + std::to_string(lista_data[rango.x + i].y) + ",";
				}

				if(!linea.empty() && linea.back() == ',')
					linea.pop_back();

				linea += "\n";

				archivo_informacion << linea;
			}
			archivo_informacion.flush();
			archivo_informacion.close();
		}
};

//Controlador porque puede controlar si la grafica esta activa o no, importante a la hora de dibujarla.
struct sControladorGrafico
{
	cGrafica * grafica;
	eEstadoGrafica estado;

	sControladorGrafico()
	{
		estado = vacia;
		grafica = nullptr;
	}
};