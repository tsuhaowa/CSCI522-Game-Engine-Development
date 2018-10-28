
-- this script is in lua format
-- this is a meta script that fills in data that is passed to 'myScript' that in turn calls C++ function
-- some of the data can be set by default, some of the data might be required to be edited from maya
function fillMetaInfoTable(args) -- the script fromat requires existance of this function

-- user modifed data
args['myScript']="NaziCar.lua"
args['myScriptPackage']="CharacterControl"

args['meshName'] = "nazicar.x_carmesh_mesh.mesha"
args['meshPackage'] = "Default"

args['patrolWayPoint'] = "10"

args['carID'] = '1'
args['frictionCoef'] = "0.5"
args['mass'] = "100"
args['horsepower'] = "60"
args['speedMax'] = "2.5"

end -- required
