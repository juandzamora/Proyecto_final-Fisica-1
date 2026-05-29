#pragma once
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

#include "grafica.h"

static float z_de_todo_el_proyecto = 0.0f;

enum eRecoleccionDatosPlaneta
{
	dat_velocidad,
	dat_aceleracion,
	dat_energia_cinetica
};

class cPlaneta
{
	private:
		//Caracteristicas del plantea
		std::string nombre_planeta;

		Vector2 velocidad;
		Vector2 aceleracion;
		//Reescalado por supuesto
		float masa;


		Vector2 posicion;
		//Para hacer que la camara se mueva con el planeta
		Vector2 posicion_anterior;

		float tiempo_orbitando;

		//Para la toma de datos
		sControladorGrafico grafica_velocidad;
		sControladorGrafico grafica_aceleracion;
		sControladorGrafico grafica_energia_cinetica;

		float valor_inicial_velocidad;
		float valor_inicial_aceleracion;
		float valor_inicial_energia_cinetica;

		unsigned int duracion_año_planeta;


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

		float angulo_anterior_vuelta; 
		float tiempo_de_vuelta_anterior;

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

		void llenarDatosGraficas()
		{
			float dia = (365.0f/2238.73f) * tiempo_orbitando;
			float magnitud_velocidad = sqrtf(velocidad.x*velocidad.x + velocidad.y*velocidad.y); 
			if(grafica_velocidad.estado == activa)
				grafica_velocidad.grafica.añadirData({dia, magnitud_velocidad});

			if(grafica_aceleracion.estado == activa)
				grafica_aceleracion.grafica.añadirData({dia, sqrtf(aceleracion.x*aceleracion.x + aceleracion.y*aceleracion.y)});

			if(grafica_energia_cinetica.estado == activa)
				grafica_energia_cinetica.grafica.añadirData({dia, (masa * magnitud_velocidad * magnitud_velocidad)/2.0f});
		}

	public:
		cPlaneta(std::string nombre_planeta, float distancia_al_sol, float masa, unsigned int duracion_año_planeta, float radio, Color color, Vector2 velocidad_inicial = {0, 0}, bool ajustar_velocidad = true, bool es_sol = false)
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

			distancia_al_sol *= factor_de_escalado_distancia;

			if(ajustar_velocidad)
			{
				float v_orbital = sqrt(1.0f / distancia_al_sol);
				this->velocidad = {0, v_orbital * 0.5f};
			}
			else
				this->velocidad = velocidad_inicial;

			this->valor_inicial_velocidad = sqrtf(velocidad.x*velocidad.x + velocidad.y*velocidad.y);

			this->masa = masa;

			this->valor_inicial_energia_cinetica = (masa * valor_inicial_velocidad*valor_inicial_velocidad)/2.0f;

			this->radio = radio * factor_de_escalado_radio;

			this->posicion = {distancia_al_sol, 0.0f};

			this->color = color;

			//Para la vuelta
			this->angulo_anterior_vuelta = 0.0f; 
			this->tiempo_de_vuelta_anterior = 0.0f;
			this->duracion_año_planeta = duracion_año_planeta;
		}

		//dt = delta de tiempo, tiempo que paso entre el anterior calculo (o llamado a esta función) a este. Es simulado, no tiene que ser realista.
		void updatePosition(double dt)
		{
			tiempo_orbitando += dt;
			actualizarAceleracion();
			
			if(tiempo_orbitando == dt)
				this->valor_inicial_aceleracion = sqrtf(aceleracion.x*aceleracion.x + aceleracion.y*aceleracion.y);

			actualizarVelocidad(dt);
			//Actualizar la posición
			posicion.x = posicion.x + velocidad.x * dt;
			posicion.y = posicion.y + velocidad.y * dt;

			modificarPath(dt);
			llenarDatosGraficas();
		}

		bool completoUnPeriodo()
		{
			float angulo = atan2(posicion.y, posicion.x);
			
			if(angulo > 0 && angulo_anterior < 0)
			{
				tiempo_de_vuelta_anterior = tiempo_orbitando;
				angulo_anterior = angulo;
				return true;
			}
			
			angulo_anterior = angulo;
			return false;
		}

		void cambiarEstadoRecoleccionDatos(eRecoleccionDatosPlaneta tipo_de_dato, eEstadoGrafica nuevo_estado)
		{
			if(tipo_de_dato == dat_velocidad && nuevo_estado == activa)
			{
				if(grafica_velocidad.estado == activa)
					return;
				grafica_velocidad.estado = activa;
				grafica_velocidad.grafica = cGrafica("Velocidad de " + nombre_planeta, std::string("Dias terrestres"), std::string("Magnitud de la velocidad (km/s)"), 
											std::string("Valor inicial de la magnitud de la velocidad"), valor_inicial_velocidad, duracion_año_planeta);
				return;
			}
			else if(tipo_de_dato == dat_velocidad && nuevo_estado == pausada)
			{
				if(grafica_velocidad.estado == pausada)
				{
					grafica_velocidad.estado = activa;
					return;
				}
				grafica_velocidad.grafica.terminarSerie();
				grafica_velocidad.estado = pausada;
			}
			else if(tipo_de_dato == dat_velocidad && nuevo_estado == vacia)
			{
				if(grafica_velocidad.estado == vacia)
					return;

				grafica_velocidad.estado = vacia;
				grafica_velocidad.grafica.limpiarData();
				return;
			}

			if(tipo_de_dato == dat_aceleracion && nuevo_estado == activa)
			{
				if(grafica_aceleracion.estado == activa)
					return;

				grafica_aceleracion.estado = activa;
				grafica_aceleracion.grafica = cGrafica("Aceleracion de " + nombre_planeta, std::string("Dias terrestres"), std::string("Magnitud de la aceleracion (km/s^2)"), 
											std::string("Valor inicial de la magnitud de la aceleracion"), valor_inicial_aceleracion, duracion_año_planeta);
				return;
			}
			else if(tipo_de_dato == dat_aceleracion && nuevo_estado == pausada)
			{
				if(grafica_aceleracion.estado == pausada)
				{
					grafica_aceleracion.estado = activa;
					return;
				}

				grafica_aceleracion.grafica.terminarSerie();
				grafica_aceleracion.estado = pausada;
			}
			else if(tipo_de_dato == dat_aceleracion && nuevo_estado == vacia)
			{
				if(grafica_aceleracion.estado == vacia)
					return;

				grafica_aceleracion.estado = vacia;
				grafica_aceleracion.grafica.limpiarData();
				return;
			}


			if(tipo_de_dato == dat_energia_cinetica && nuevo_estado == activa)
			{
				if(grafica_energia_cinetica.estado == activa)
					return;

				grafica_energia_cinetica.estado = activa;
				grafica_energia_cinetica.grafica = cGrafica("Energia cinetica de " + nombre_planeta, std::string("Dias terrestres"), std::string("Energia cinetica ((kg*km^2)/s^2)"), 
											std::string("Valor inicial de la energia cinetica"), valor_inicial_energia_cinetica, duracion_año_planeta);
				return;
			}
			else if(tipo_de_dato == dat_energia_cinetica && nuevo_estado == pausada)
			{
				if(grafica_energia_cinetica.estado == pausada)
				{
					grafica_energia_cinetica.estado = activa;
					return;
				}
				grafica_energia_cinetica.grafica.terminarSerie();
				grafica_energia_cinetica.estado = pausada;
			}
			else if(tipo_de_dato == dat_energia_cinetica && nuevo_estado == vacia)
			{
				if(grafica_energia_cinetica.estado == vacia)
					return;

				grafica_energia_cinetica.estado = vacia;
				grafica_energia_cinetica.grafica.limpiarData();
				return;
			}

		}
		
		void verGraficoVelocidad()
		{
			if((grafica_velocidad.estado == activa || grafica_velocidad.estado == pausada) && !grafica_velocidad.grafica.obtenerData().empty())
				grafica_velocidad.grafica.visualizarGrafica();
		}

		void verGraficoAceleracion()
		{
			if((grafica_aceleracion.estado == activa || grafica_aceleracion.estado == pausada) && !grafica_aceleracion.grafica.obtenerData().empty())
				grafica_aceleracion.grafica.visualizarGrafica();
		}

		void verGraficoEnergiaCinetica()
		{
			if((grafica_energia_cinetica.estado == activa || grafica_energia_cinetica.estado == pausada) && !grafica_energia_cinetica.grafica.obtenerData().empty())
				grafica_energia_cinetica.grafica.visualizarGrafica();
		}

		eEstadoGrafica getEstadoGrafica(eRecoleccionDatosPlaneta tipo_de_dato)
		{
			if(tipo_de_dato == dat_velocidad)
				return grafica_velocidad.estado;

			if(tipo_de_dato == dat_aceleracion)
				return grafica_aceleracion.estado;

			if(tipo_de_dato == dat_energia_cinetica)
				return grafica_energia_cinetica.estado;
		}

		void guardarGrafica(eRecoleccionDatosPlaneta tipo_de_dato)
		{
			if(tipo_de_dato == dat_velocidad && grafica_velocidad.estado == pausada)
				return grafica_velocidad.grafica.guardarInformacion();

			if(tipo_de_dato == dat_aceleracion && grafica_aceleracion.estado == pausada)
				return grafica_aceleracion.grafica.guardarInformacion();

			if(tipo_de_dato == dat_energia_cinetica && grafica_energia_cinetica.estado == pausada)
				return grafica_energia_cinetica.grafica.guardarInformacion();
		}

		bool getSol()
		{
			return es_sol;
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

		float getTiempoVueltaAnterior()
		{
			return tiempo_de_vuelta_anterior;
		}
};
