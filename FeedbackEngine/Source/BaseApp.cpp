#include "BaseApp.h"

HRESULT 
BaseApp::init() {
	HRESULT hr = S_OK;

	// Create Swapchain and BackBuffer
	hr = m_swapchain.init(m_device, m_deviceContext, m_backBuffer, m_window);
	if (FAILED(hr)) {
		return hr;
	}

	// Create a render target view
	hr = m_renderTargetView.init(m_device,
		m_backBuffer,
		DXGI_FORMAT_R8G8B8A8_UNORM);

	if (FAILED(hr)) {
		return hr;
	}

	// Create a depth stencil
	hr = m_depthStencil.init(m_device,
		m_window.m_width,
		m_window.m_height,
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		D3D11_BIND_DEPTH_STENCIL,
		4,
		0);
	if (FAILED(hr))
		return hr;

	// Create the depth stencil view
	hr = m_depthStencilView.init(m_device,
		m_depthStencil,
		DXGI_FORMAT_D24_UNORM_S8_UINT);

	if (FAILED(hr))
		return hr;

	// Setup the viewport
	hr = m_viewport.init(m_window);

	if (FAILED(hr))
		return hr;

	// Define the input layout
	std::vector<D3D11_INPUT_ELEMENT_DESC> Layout;

	D3D11_INPUT_ELEMENT_DESC position;
	position.SemanticName = "POSITION";
	position.SemanticIndex = 0;
	position.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	position.InputSlot = 0;
	position.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT /*0*/;
	position.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	position.InstanceDataStepRate = 0;
	Layout.push_back(position);

	D3D11_INPUT_ELEMENT_DESC texcoord;
	texcoord.SemanticName = "TEXCOORD";
	texcoord.SemanticIndex = 0;
	texcoord.Format = DXGI_FORMAT_R32G32_FLOAT;
	texcoord.InputSlot = 0;
	texcoord.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT /*12*/;
	texcoord.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	texcoord.InstanceDataStepRate = 0;
	Layout.push_back(texcoord);

	// Create the Shader Program
	hr = m_shaderProgram.init(m_device, "FeedbackEngine.fx", Layout);

	if (FAILED(hr))
		return hr;
	
	// Create the constant buffers
	hr = m_neverChanges.init(m_device, sizeof(CBNeverChanges));
	if (FAILED(hr))
		return hr;

	hr = m_changeOnResize.init(m_device, sizeof(CBChangeOnResize));
	if (FAILED(hr))
		return hr;

	hr = m_imguiTexture.init(m_device, 
													 m_window.m_width,
													 m_window.m_height,
													 DXGI_FORMAT_R8G8B8A8_UNORM,
													 D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
													 1,
													 0);

	if (FAILED(hr)) {
		return hr;
	}
	

	// Crear una vista de render target para la textura de IMGUI
	hr = m_imguiRenderTargetView.init(m_device,
																		m_imguiTexture,
																		D3D11_RTV_DIMENSION_TEXTURE2D, //D3D11_RTV_DIMENSION_TEXTURE2DMS
																		DXGI_FORMAT_R8G8B8A8_UNORM);

	if (FAILED(hr)) {
		return hr;
	}

	hr = m_imguiShaderResourceView.init(m_device, m_imguiTexture, DXGI_FORMAT_R8G8B8A8_UNORM);
	if (FAILED(hr)) {
		return hr;
	}

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_View = XMMatrixLookAtLH(Eye, At, Up);

	// Initialize User Interface
	m_userInterface.init(m_window.m_hWnd,
		m_device.m_device,
		m_deviceContext.m_deviceContext);


	// Set Vela Actor
	// Load the Texture
	Texture Vela_Char_BaseColor;
	Vela_Char_BaseColor.init(m_device,
		"Textures/Vela/Vela_Char_BaseColor.png",
		ExtensionType::PNG);

	Texture Vela_Corneas_BaseColor;
	Vela_Corneas_BaseColor.init(m_device,
		"Textures/Vela/Vela_Corneas_BaseColor.png",
		ExtensionType::PNG);

	Texture Vela_Gun_BaseColor;
	Vela_Gun_BaseColor.init(m_device,
		"Textures/Vela/Vela_Gun_BaseColor.png",
		ExtensionType::PNG);

	Texture Vela_Legs_BaseColor;
	Vela_Legs_BaseColor.init(m_device,
		"Textures/Vela/Vela_Legs_BaseColor.png",
		ExtensionType::PNG);

	Texture Vela_Mechanical_BaseColor;
	Vela_Mechanical_BaseColor.init(m_device,
		"Textures/Vela/Vela_Mechanical_BaseColor.png",
		ExtensionType::PNG);

	Texture Vela_Plate_BaseColor;
	Vela_Plate_BaseColor.init(m_device,
		"Textures/Vela/Vela_Plate_BaseColor.png",
		ExtensionType::PNG);

	Texture Vela_Visor_BaseColor;
	Vela_Visor_BaseColor.init(m_device,
		"Textures/Vela/Vela_Visor_BaseColor.png",
		ExtensionType::PNG);
	
	m_default.init(m_device, "Textures/Default.png", ExtensionType::PNG);

	m_velaTextures.push_back(Vela_Corneas_BaseColor);			// 1 
	m_velaTextures.push_back(Vela_Gun_BaseColor);					// 2
	m_velaTextures.push_back(Vela_Visor_BaseColor);				// 3
	m_velaTextures.push_back(Vela_Legs_BaseColor);				// 4
	m_velaTextures.push_back(Vela_Mechanical_BaseColor);	// 5
	m_velaTextures.push_back(Vela_Char_BaseColor);				// 6
	m_velaTextures.push_back(Vela_Plate_BaseColor);				// 7
	m_velaTextures.push_back(m_default);				// 7
	// Load Model
	m_Vela.LoadFBXModel("Models/Vela2.fbx");
	AVela = EngineUtilities::MakeShared<Actor>(m_device);
	if (!AVela.isNull()) {
		// Init Actor Transform
		AVela->getComponent<Transform>()->setTransform(EngineUtilities::Vector3(-0.9f, -2.0f, 2.0f), 
																									 EngineUtilities::Vector3(XM_PI / -2.0f, 0.0f, XM_PI / 2.0f),
																									 EngineUtilities::Vector3(0.03f, 0.03f, 0.03f));
		// Init Actor Mesh
		AVela->setMesh(m_device, m_Vela.meshes);
		// Init Actor Textures
		AVela->setTextures(m_velaTextures);

		std::string msg = AVela->getName() + " - Actor accessed successfully.";
		MESSAGE("Actor", "Actor", msg.c_str());
	}
	else {
		MESSAGE("Actor", "Actor", "Actor resource not found.");
	}


	return S_OK;
}

void 
BaseApp::update() {
	m_userInterface.update();
	bool show_demo_window = true;
	ImGui::ShowDemoWindow(&show_demo_window);
	// Actualizar tiempo y rotación
	static float t = 0.0f;
	if (m_swapchain.m_driverType == D3D_DRIVER_TYPE_REFERENCE) {
		t += (float)XM_PI * 0.0125f;
	}
	else {
		static DWORD dwTimeStart = 0;
		DWORD dwTimeCur = GetTickCount();
		if (dwTimeStart == 0)
			dwTimeStart = dwTimeCur;
		t = (dwTimeCur - dwTimeStart) / 1000.0f;
	}

	updateTranslationByKey(t);

	// Actualizar la matriz de proyección
	// Matriz de proyección con FOV similar a Unreal (90°)
	float FOV = XMConvertToRadians(90.0f);
	m_Projection = XMMatrixPerspectiveFovLH(FOV, m_window.m_width / (float)m_window.m_height, 0.01f, 10000.0f);

	updateCamera();
	// Actualizar la proyección en el buffer constante
	cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);
	m_changeOnResize.update(m_deviceContext, 0, nullptr, &cbChangesOnResize, 0, 0);

	// Actualizar info logica del mesh
	AVela->update(0, m_deviceContext);

	ImGui::Begin("Transform");

	// Draw the structure
	m_userInterface.vec3Control("Position", const_cast<float*>(AVela->getComponent<Transform>()->getPosition().data()));
	m_userInterface.vec3Control("Rotation", const_cast<float*>(AVela->getComponent<Transform>()->getRotation().data()));
	m_userInterface.vec3Control("Scale", const_cast<float*>(AVela->getComponent<Transform>()->getScale().data()));

	ImGui::End();
}

void 
BaseApp::render() {
	// Limpiar los buffers
	const float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha

	// Configurar la textura IMGUI como la vista de renderizado
	m_imguiRenderTargetView.render(m_deviceContext, 1);
	// Set Render Target View
	m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);

	// Set Viewport
	m_viewport.render(m_deviceContext);

	// Set Depth Stencil View
	m_depthStencilView.render(m_deviceContext);

	// Configurar los buffers y shaders para el pipeline
	m_shaderProgram.render(m_deviceContext);

	// Render the models
	AVela->render(m_deviceContext);

	// Asignar shaders y buffers constantes
	// Renderizar buffers constantes en el Vertex Shader
	m_neverChanges.render(m_deviceContext, 0, 1);
	m_changeOnResize.render(m_deviceContext, 1, 1);

	// Copiar el back buffer a la textura IMGUI
	m_swapchain.m_swapchain->GetBuffer(0,
		__uuidof(ID3D11Texture2D),
		reinterpret_cast<void**>(&m_backBuffer.m_texture));

	// Si el backbuffer es multisample, resolverlo a una textura sin MSAA (la usada por ImGui)
	m_deviceContext.m_deviceContext->ResolveSubresource(
		m_imguiTexture.m_texture,      // Destino: textura de ImGui
		0,                             // Mip level
		m_backBuffer.m_texture,        // Fuente: backbuffer
		0,                             // Mip level fuente
		DXGI_FORMAT_R8G8B8A8_UNORM);    // Formato

	// Renderizar la interfaz de usuario y mostrar la imagen
	m_userInterface.Renderer(m_window, m_imguiShaderResourceView.m_textureFromImg);
	m_userInterface.render();

	// Presentar el frame en pantalla
	m_swapchain.present();
}

void 
BaseApp::destroy() {
	if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();

	AVela->destroy();

	m_neverChanges.destroy();
	m_changeOnResize.destroy();
	m_changeEveryFrame.destroy();
	m_shaderProgram.destroy();

	m_depthStencil.destroy();
	m_depthStencilView.destroy();
	m_renderTargetView.destroy();
	m_swapchain.destroy();
	m_deviceContext.destroy();
	m_userInterface.destroy();
	m_device.destroy();
}

HRESULT 
BaseApp::resizeWindow(HWND hWnd, LPARAM lParam) {
	if (m_swapchain.m_swapchain) {
		unsigned int newWidth = LOWORD(lParam);
		unsigned int newHeight = HIWORD(lParam);

		if (newWidth == 0 || newHeight == 0) {
			return S_OK;
		}

		m_window.m_width = LOWORD(lParam);
		m_window.m_height = HIWORD(lParam);

		// Libera los recursos existentes
		m_renderTargetView.destroy();
		m_depthStencilView.destroy();
		m_depthStencil.destroy();
		m_backBuffer.destroy();
		m_imguiRenderTargetView.destroy();
		m_imguiShaderResourceView.destroy();
		m_imguiTexture.destroy();

		// Redimensionar el swap chain
		HRESULT hr = m_swapchain.m_swapchain->ResizeBuffers(0,
																												m_window.m_width,
																												m_window.m_height,
																												DXGI_FORMAT_R8G8B8A8_UNORM,
																												0);
		if (FAILED(hr)) {
			// Si falla, puede ser útil obtener la razón del dispositivo removido, por ejemplo:
			if (hr == DXGI_ERROR_DEVICE_REMOVED) {
				HRESULT reason = m_device.m_device->GetDeviceRemovedReason();
				// Puedes imprimir o loggear "reason" para depuración.
			}
			MessageBox(hWnd, "Failed to resize swap chain buffers.", "Error", MB_OK);
			PostQuitMessage(0);
		}

		// **3. RECREAR EL BACK BUFFER**
		hr = m_swapchain.m_swapchain->GetBuffer(0,
			__uuidof(ID3D11Texture2D),
			reinterpret_cast<void**>(&m_backBuffer.m_texture));
		if (FAILED(hr)) {
			ERROR("SwapChain", "Resize", "Failed to get new back buffer");
			return hr;
		}

		// **4. RECREAR EL RENDER TARGET VIEW**
		hr = m_renderTargetView.init(m_device,
			m_backBuffer,
			DXGI_FORMAT_R8G8B8A8_UNORM);
		if (FAILED(hr)) {
			ERROR("RenderTargetView", "Resize", "Failed to create new RenderTargetView");
			return hr;
		}

		// **5. RECREAR EL DEPTH STENCIL**
		hr = m_depthStencil.init(m_device,
			m_window.m_width,
			m_window.m_height,
			DXGI_FORMAT_D24_UNORM_S8_UINT,
			D3D11_BIND_DEPTH_STENCIL,
			4,
			0);
		if (FAILED(hr)) {
			ERROR("DepthStencil", "Resize", "Failed to create new DepthStencil");
			return hr;
		}

		// **6. RECREAR EL DEPTH STENCIL VIEW**

		hr = m_depthStencilView.init(m_device,
			m_depthStencil,
			DXGI_FORMAT_D24_UNORM_S8_UINT);
		if (FAILED(hr)) {
			ERROR("DepthStencilView", "Resize", "Failed to create new DepthStencilView");
			return hr;
		}

		// Recrear la textura para ImGui con las nuevas dimensiones
		hr = m_imguiTexture.init(m_device,
			m_window.m_width,
			m_window.m_height,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
			1,
			0);
		if (FAILED(hr)) {
			ERROR("ImGuiTexture", "Resize", "Failed to create new ImGui texture");
			return hr;
		}

		// Recrear la vista de render target para la textura de ImGui
		hr = m_imguiRenderTargetView.init(m_device,
			m_imguiTexture,
			D3D11_RTV_DIMENSION_TEXTURE2D,
			DXGI_FORMAT_R8G8B8A8_UNORM);
		if (FAILED(hr)) {
			ERROR("ImGuiRTV", "Resize", "Failed to create new ImGui RenderTargetView");
			return hr;
		}

		// Recrear el Shader Resource View para la textura de ImGui
		hr = m_imguiShaderResourceView.init(m_device,
			m_imguiTexture,
			DXGI_FORMAT_R8G8B8A8_UNORM);
		if (FAILED(hr)) {
			ERROR("ImGuiSRV", "Resize", "Failed to create new ImGui ShaderResourceView");
			return hr;
		}

		// Actualizar el viewport
		hr = m_viewport.init(m_window);

		if (FAILED(hr)) {
			ERROR("Viewport", "Resize", "Failed to create new Viewport");
			return hr;
		}

		// Actualizar la proyección
		m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_window.m_width / (float)m_window.m_height, 0.01f, 100.0f);
		cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);
		m_changeOnResize.update(m_deviceContext, 0, nullptr, &cbChangesOnResize, 0, 0);
	}
}

void 
BaseApp::updateTranslationByKey(float deltaTime) {
	float moveSpeed = 0.001f; // Velocidad de movimiento
	float moveSpeedCamera = 0.01f; // Velocidad de movimiento

	//if (keys[VK_UP])    position.y += moveSpeed * deltaTime; // Mueve arriba
	//if (keys[VK_DOWN])  position.y -= moveSpeed * deltaTime; // Mueve abajo
	//if (keys[VK_LEFT])  position.x -= moveSpeed * deltaTime; // Mueve izquierda
	//if (keys[VK_RIGHT]) position.x += moveSpeed * deltaTime; // Mueve derecha
	//if (keys['E'])      position.z += moveSpeed * deltaTime; // Avanza en Z
	//if (keys['Q'])      position.z -= moveSpeed * deltaTime; // Retrocede en Z

	XMVECTOR pos = XMLoadFloat3(&m_camera.position);
	XMVECTOR forward = XMLoadFloat3(&m_camera.forward);
	XMVECTOR right = XMLoadFloat3(&m_camera.right);

	if (keys['W']) pos += forward * moveSpeedCamera;
	if (keys['S']) pos -= forward * moveSpeedCamera;
	if (keys['A']) pos -= right * moveSpeedCamera;
	if (keys['D']) pos += right * moveSpeedCamera;

	XMStoreFloat3(&m_camera.position, pos);

}

void 
BaseApp::updateCamera() {
	// Convertir la dirección a vectores normalizados
	XMVECTOR pos = XMLoadFloat3(&m_camera.position);
	XMVECTOR dir = XMLoadFloat3(&m_camera.forward);
	XMVECTOR up = XMLoadFloat3(&m_camera.up);

	// Calcular la nueva vista
	m_View = XMMatrixLookAtLH(pos, pos + dir, up);

	// Transponer y actualizar el buffer de la vista
	cbNeverChanges.mView = XMMatrixTranspose(m_View);
	m_neverChanges.update(m_deviceContext, 0, nullptr, &cbNeverChanges, 0, 0);
}

void 
BaseApp::rotateCamera(int mouseX, int mouseY) {
	float offsetX = (mouseX - lastX) * sensitivity;
	float offsetY = (mouseY - lastY) * sensitivity;
	lastX = mouseX;
	lastY = mouseY;

	m_camera.yaw += offsetX;
	m_camera.pitch += offsetY;

	// Limitar la inclinación de la cámara
	if (m_camera.pitch > 1.5f) m_camera.pitch = 1.5f;
	if (m_camera.pitch < -1.5f) m_camera.pitch = -1.5f;

	// Recalcular la dirección hacia adelante
	XMVECTOR forward = XMVectorSet(
		cosf(m_camera.yaw) * cosf(m_camera.pitch),
		sinf(m_camera.pitch),
		sinf(m_camera.yaw) * cosf(m_camera.pitch),
		0.0f
	);

	XMVECTOR right = XMVector3Cross(forward, XMLoadFloat3(&m_camera.up));

	XMStoreFloat3(&m_camera.forward, XMVector3Normalize(forward));
	XMStoreFloat3(&m_camera.right, XMVector3Normalize(right));
}

int 
BaseApp::run(HINSTANCE hInstance, 
						 HINSTANCE hPrevInstance, 
						 LPWSTR lpCmdLine, 
						 int nCmdShow, 
						 WNDPROC wndproc) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(m_window.init(hInstance, nCmdShow, wndproc)))
		return 0;

	if (FAILED(init())) {
		destroy();
		return 0;
	}

	// Main message loop
	MSG msg = { 0 };
	while (WM_QUIT != msg.message) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			update();
			render();
		}
	}

	destroy();

	return (int)msg.wParam;
}
