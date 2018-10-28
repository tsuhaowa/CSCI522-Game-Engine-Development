-- this is game object script for a nazicar model
-- this game object represents a basic nazicar model that is added to the scene
outputDebugString("Executing NaziCar.lua\n")
function runScript(args)
    handler = getGameObjectManagerHandle(l_getGameContext())
    local pos = args['base']['pos']
    local u = args['base']['u']
    local v = args['base']['v']
    local n = args['base']['n']
    
    evt = root.CharacterControl.Events.Event_CREATE_NAZICAR.Construct(
    l_getGameContext(),
    args['meshName'], args['meshPackage'], args['patrolWayPoint'],
	args['carID'], args['frictionCoef'], args['mass'],
	args['horsepower'], args['speedMax'],
    pos[1], pos[2], pos[3],
    u[1], u[2], u[3],
    v[1], v[2], v[3],
    n[1], n[2], n[3],
    args['peuuid'])
    root.PE.Components.Component.SendEventToHandle(handler, evt)
end
