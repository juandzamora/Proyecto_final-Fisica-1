#pragma once
#include <cmath>
using std::sin;
using std::cos;
using std::acos;
using std::pow;
using std::sqrt;
#include <vector>
#include <string>

#include <raylib.h>
#include <raymath.h>

#include <rlImGui.h>
#include <rlImGuiColors.h>
#include <imgui.h>

#include "planeta.h"

class cCamara3DCustom
{
	private:
		Camera3D camara;

		//Los angulo en X e Y de la camara
		float angulo_y;
		float angulo_x;

		const float epsilon = 0.01f;

		bool mantener_fija_en_planeta;
		int id_planeta_fijo;
		float factor_zoom_cuando_fijo;
		//ultimo_cambio_zoom guarda el valor del ultimo cambio a zoom para restarselo a factor_zoom_cuando_fijo en caso de que la hipotenusa se vuelva 0
		float ultimo_cambio_zoom;
		float factor_anguloX_cuando_fijo;
		float factor_anguloY_cuando_fijo;

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

			float cambio_en_angulo_x = mouse_delta.x * sensibilidadX;
			float cambio_en_angulo_y = mouse_delta.y * sensibilidadY;

			if(mantener_fija_en_planeta)
			{
				factor_anguloX_cuando_fijo += cambio_en_angulo_x;
				factor_anguloY_cuando_fijo += cambio_en_angulo_y;
				return;
			}

			angulo_x += cambio_en_angulo_x;
			angulo_y = Clamp(angulo_y + cambio_en_angulo_y, 0 + epsilon, PI - epsilon);


			actualizarPosicion(getHipotenusa());
		}

		void fijarFirmeAPlaneta(Vector3 posicion_planeta, Vector3 posicion_fija, float radio_planeta)
		{
			camara.target = posicion_planeta;

			camara.position = posicion_fija;

			float hipotenusa = sqrt(pow(posicion_fija.x, 2) + pow(posicion_fija.y, 2) + pow(posicion_fija.z, 2)) + this->factor_zoom_cuando_fijo;

			if(hipotenusa < radio_planeta * 2)
			{
				hipotenusa -= this->ultimo_cambio_zoom;

				this->factor_zoom_cuando_fijo -= this->ultimo_cambio_zoom;
				this->ultimo_cambio_zoom = 0.0f;
			}

			
			//Movimiento con mouse cuando se esta firme a un planeta
			angulo_y = acos((posicion_fija.y - camara.target.y)/hipotenusa);
			angulo_x = atan2(posicion_fija.z - camara.target.z, posicion_fija.x - camara.target.x) + factor_anguloX_cuando_fijo;

			angulo_y = Clamp(angulo_y + factor_anguloY_cuando_fijo, 0 + epsilon, PI - epsilon);

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

			//Variables para fijar en planeta
			this->mantener_fija_en_planeta = false;
			this->id_planeta_fijo = -1;
			this->factor_zoom_cuando_fijo = 0.0f;
			this->ultimo_cambio_zoom = 0.0f;
			this->factor_anguloX_cuando_fijo = 0.0f;
			this->factor_anguloY_cuando_fijo = 0.0f;
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
				{
					this->factor_zoom_cuando_fijo += cambio * -1;
					this->ultimo_cambio_zoom = cambio * -1;
				}
				
				if(IsKeyDown(KEY_S))
				{
					this->factor_zoom_cuando_fijo += cambio;
					this->ultimo_cambio_zoom = cambio;
				}
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
		
		//El id tiene que ser positivo
		void cambiarPlanetaTarget(cPlaneta & planeta, int id_planeta, bool mantener_fija_en_planeta)
		{
			if(id_planeta < 0)
				return;

			if(mantener_fija_en_planeta)
			{
				if(this->id_planeta_fijo != id_planeta)
				{
					this->factor_zoom_cuando_fijo = 0.0f;
					this->ultimo_cambio_zoom = 0.0f;
					this->factor_anguloX_cuando_fijo = 0.0f;
					this->factor_anguloY_cuando_fijo = 0.0f;
					this->id_planeta_fijo = id_planeta;
				}

				float radio_planeta = planeta.getRadio();

				float posicion_fija_coordenadas = radio_planeta/0.5f;

				Vector3 posicion_fija =  {posicion_fija_coordenadas, posicion_fija_coordenadas, posicion_fija_coordenadas};

				fijarFirmeAPlaneta(planeta.getPosicion(), posicion_fija, radio_planeta);
				this->mantener_fija_en_planeta = true;

				return;
			}

			this->mantener_fija_en_planeta = false;

			camara.target = planeta.getPosicion();

			float hipotenusa = getHipotenusa();

			angulo_y = acos(camara.position.y/hipotenusa);
			angulo_x = atan2(camara.position.z - camara.target.z, camara.position.x - camara.target.x);

			actualizarPosicion(hipotenusa);
		}
};
