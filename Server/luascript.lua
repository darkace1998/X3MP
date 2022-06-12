function onScriptStart()
	print("Testgamemode started test...")
	createShip(22);
end

function onPlayerConnect(clientID)
	print("Client id " .. clientID .. " connected")
end

function onScriptStop()
	print("Testgamemode stopped")
end