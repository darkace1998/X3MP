package game

// Entity represents an object in the game world, like a ship.
// The fields are based on the C++ Entity and packet structures.
type Entity struct {
	Model     int32
	NetOwnerID int32 // The ClientID of the player who owns this entity
	Owner     int32 // In-game owner (not used yet)

	// Positional and rotational data
	PosX int32
	PosY int32
	PosZ int32
	RotX int32
	RotY int32
	RotZ int32
	RotW int32
	UpX  int32
	UpY  int32
	UpZ  int32
	UpW  int32
	LookAtX int32
	LookAtY int32
	LookAtZ int32
}

// Universe holds the entire state of the game world.
type Universe struct {
	Entities map[int32]*Entity // Map of ShipID to Entity
}

// NewUniverse creates a new, empty universe.
func NewUniverse() *Universe {
	return &Universe{
		Entities: make(map[int32]*Entity),
	}
}

// CreateShip finds the next available ShipID, creates a ship, and returns its ID.
func (u *Universe) CreateShip(model int32, ownerClientID int32) (int32, *Entity) {
	// Find the next available ID.
	// In the C++ code, it iterates from 0 to 65535.
	// A map makes this slightly different, but we can replicate the logic.
	var nextID int32
	for i := int32(0); i < 65535; i++ {
		if _, exists := u.Entities[i]; !exists {
			nextID = i
			break
		}
	}

	entity := &Entity{
		Model:     model,
		NetOwnerID: ownerClientID,
	}
	u.Entities[nextID] = entity
	return nextID, entity
}

// DeleteShip removes a ship from the universe.
func (u *Universe) DeleteShip(shipID int32) {
	delete(u.Entities, shipID)
}
