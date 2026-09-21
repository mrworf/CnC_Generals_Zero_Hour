// Original DX8Wrapper source light expansion, shared by native DX8 and the
// mutually exclusive Linux CPU/device-edge configuration.
void DX8Wrapper::Set_Light_Environment(LightEnvironmentClass* light_env)
{
	// Shader light environment support
//	if (Light_Environment && light_env && (*Light_Environment)==(*light_env)) return;
#if defined(ZH_WW3D_CPU_ONLY)
	zh::original_runtime::OriginalGpuEdge::required();
	if (light_env) {
		if (light_env->Get_Light_Count()<0 || light_env->Get_Light_Count()>4)
			throw std::runtime_error("original source light count exceeds four slots");
		const Vector3& ambient=light_env->Get_Equivalent_Ambient();
		if (!std::isfinite(ambient.X)||!std::isfinite(ambient.Y)||!std::isfinite(ambient.Z)||
			ambient.X<0||ambient.X>1||ambient.Y<0||ambient.Y>1||ambient.Z<0||ambient.Z>1)
			throw std::runtime_error("original source ambient exceeds normalized physical profile");
		for (int i=0;i<light_env->Get_Light_Count();++i) {
			const Vector3& diffuse=light_env->Get_Light_Diffuse(i);
			const Vector3& direction=light_env->Get_Light_Direction(i);
			const auto finite=[](const Vector3 &v) {
				return std::isfinite(v.X)&&std::isfinite(v.Y)&&std::isfinite(v.Z);
			};
			const float inner=light_env->getPointIrad(i);
			const float outer=light_env->getPointOrad(i);
			if (!std::isfinite(diffuse.X)||!std::isfinite(diffuse.Y)||!std::isfinite(diffuse.Z)||
				!std::isfinite(direction.X)||!std::isfinite(direction.Y)||!std::isfinite(direction.Z)||
				(light_env->isPointLight(i) &&
				(!std::isfinite(inner)||!std::isfinite(outer)||outer<=0||
				 !finite(light_env->getPointCenter(i))||
				 !finite(light_env->getPointAmbient(i))||
				 !finite(light_env->getPointDiffuse(i))||
				 (std::fabs(inner-outer)>=1e-5 &&
				  (inner<=0 || !std::isfinite(static_cast<float>(0.1/inner))))||
				 !std::isfinite(static_cast<float>(8.0/(double(outer)*outer))))))
				throw std::runtime_error("original source light is outside bounded physical profile");
		}
	}
	state().light_environment=light_env;
	zh::original_runtime::OriginalGpuEdge::required().record_source_state(
		"DX8Wrapper::Set_Light_Environment");
#else
	Light_Environment=light_env;
#endif
	if (light_env)
	{
		int light_count = light_env->Get_Light_Count();
		unsigned int color=Convert_Color(light_env->Get_Equivalent_Ambient(),0.0f);
#if defined(ZH_WW3D_CPU_ONLY)
		if (state().render_states[D3DRS_AMBIENT]!=color)
#else
		if (RenderStates[D3DRS_AMBIENT]!=color)
#endif
		{
			Set_DX8_Render_State(D3DRS_AMBIENT,color);
//buggy Radeon 9700 driver doesn't apply new ambient unless the material also changes.
#if !defined(ZH_WW3D_CPU_ONLY)
			render_state_changed|=MATERIAL_CHANGED;
#endif
		}

		D3DLIGHT8 light;
		int l=0;
		for (;l<light_count;++l) {
			memset(&light,0,sizeof(D3DLIGHT8));
			light.Type=D3DLIGHT_DIRECTIONAL;
			(Vector3&)light.Diffuse=light_env->Get_Light_Diffuse(l);
			Vector3 dir=-light_env->Get_Light_Direction(l);
			light.Direction=(const D3DVECTOR&)(dir);
			// (gth) TODO: put specular into LightEnvironment.
			if (l==0) light.Specular.r=light.Specular.g=light.Specular.b=1.0f;
			if (light_env->isPointLight(l)) {
				light.Type=D3DLIGHT_POINT;
				(Vector3&)light.Diffuse=light_env->getPointDiffuse(l);
				(Vector3&)light.Ambient=light_env->getPointAmbient(l);
				light.Position=(const D3DVECTOR&)light_env->getPointCenter(l);
				light.Range=light_env->getPointOrad(l);
				double b=light_env->getPointOrad(l);
				double a=light_env->getPointIrad(l);
				// Original Generals correction for otherwise over-bright point lights.
				light.Attenuation0=1.0f;
				if (fabs(a-b)<1e-5) light.Attenuation1=0.0f;
				else light.Attenuation1=(float)0.1/a;
				light.Attenuation2=8.0f/(b*b);
			}
			Set_Light(l,&light);
		}
		for (;l<4;++l) Set_Light(l,NULL);
	}
	// The original null branch intentionally leaves existing global lights set.
}
