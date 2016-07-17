# include <Siv3D.hpp>

ImageR32F CreateHeightImage(const FilePath& path, float& min, float& max)
{
	const CSVReader csv(path);

	ImageR32F image(257, 257);

	size_t i = 0;

	for (auto& pixel : image)
	{
		float h = csv.getOr(0, i++, 0.0f);

		if (h > 0.0)
		{
			min = Min(h, min);

			max = Max(h, max);
		}
		else
		{
			h = -0.0005f;
		}

		pixel.r = h;
	}

	return image;
}

void Main()
{
	Window::Resize(1280, 720);
	Graphics::SetBackground(Color(40, 150, 255));
	Graphics3D::SetAmbientLight(ColorF(0.2, 0.3, 0.4));
	Graphics3D::SetFog(Fog::SquaredExponential(Color(40, 150, 255), 0.002));
	Graphics3D::SetFogForward(Fog::SquaredExponential(Color(40, 150, 255), 0.002));

	const VertexShader vsTerrain(L"Example/Shaders/Terrain3D.hlsl");
	const PixelShader psTerrain(L"Example/Shaders/Terrain3D.hlsl");
	if (!vsTerrain || !psTerrain)
	{
		return;
	}

	const Texture textureTerrain(Image(16, 16, Color(70, 150, 70)));
	const Mesh terrainBase(MeshData::TerrainBase(160, 257));	
	
	float min = 1.0f, max = 0.0f;
	DynamicTexture heightMap(CreateHeightImage(L"miyake.csv", min, max));

	GUI gui(GUIStyle::Default);
	gui.setTitle(L"海水位");
	gui.add(L"h", GUISlider::Create(0.0, 1.1, 0.0, 400));
	gui.add(L"load", GUIButton::Create(L"Load"));

	Camera camera;
	camera.lookat.set(0, 30, 79);
	camera.pos.set(0, 45, -80);
	Graphics3D::SetCamera(camera);

	while (System::Update())
	{
		if (gui.button(L"load").pushed)
		{
			if (const auto path = Dialog::GetOpen({ ExtensionFilter::CSV }))
			{
				min = 1.0f;
				max = 0.0f;
				heightMap.fill(CreateHeightImage(path.value(), min, max));
			}
		}

		Graphics3D::FreeCamera();

		Graphics3D::BeginVS(vsTerrain);
		Graphics3D::BeginPS(psTerrain);
		Graphics3D::SetTexture(ShaderStage::Vertex, 0, heightMap);
		Graphics3D::SetSamplerState(ShaderStage::Vertex, 0, SamplerState::ClampLinear);

		terrainBase.draw(textureTerrain);

		Graphics3D::SetTexture(ShaderStage::Vertex, 0, none);
		Graphics3D::EndPS();
		Graphics3D::EndVS();

		Plane({ 0, min + gui.slider(L"h").value * 5 * max,0 }, 320).drawForward(ColorF(0.2, 0.4, 1.0, 0.8));
	}
}
