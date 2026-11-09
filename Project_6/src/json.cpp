#include "json.hpp"

void Json::handle_json_from_client_task(void* parameter)
{
	auto* camera = static_cast<Json*>(parameter);
}

void Json::start_json_server(int port)
{
  	// Return early on no WiFi connection
	if (WiFi.status() != WL_CONNECTED)
	{
		return;
	}

	// Start server
	m_json_server = WiFiServer(port);
	m_json_server.begin();
}

void Json::update_json()
{
	// m_json["buttonPresses"]	= buttonPresses;
	// m_json["buttonPressTime"] = buttonPressTime;
}