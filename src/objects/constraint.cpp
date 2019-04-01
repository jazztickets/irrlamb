/******************************************************************************
* irrlamb - https://github.com/jazztickets/irrlamb
* Copyright (C) 2019  Alan Witkowski
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/
#include <objects/constraint.h>
#include <globals.h>
#include <physics.h>
#include <objects/template.h>
#include <ode/objects.h>

// Constructor
_Constraint::_Constraint(const _ConstraintSpawn &Object) :
	_Object(Object.Template),
	Joint(nullptr) {

	// Create joint
	if(Physics.IsEnabled() && Template) {
		switch(Template->Type) {
			case CONSTRAINT_HINGE: {
				if(Object.BodyA) {
					Joint = dJointCreateHinge(Physics.GetWorld(), 0);
					dJointAttach(Joint, Object.BodyA->GetBody(), 0);
					dJointSetHingeAxis(Joint, Template->ConstraintAxis[0], Template->ConstraintAxis[1], Template->ConstraintAxis[2]);

					glm::vec3 Position = Object.BodyA->GetPosition();
					dJointSetHingeAnchor(Joint, Position[0], Position[1], Position[2]);
				}
			} break;
		}
	}

	// Set basic properties
	SetProperties(Object);

	// Set user data
	if(Joint)
		dJointSetData(Joint, this);
}

// Destructor
_Constraint::~_Constraint() {
	if(Joint)
		dJointDestroy(Joint);
}
