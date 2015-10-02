/******************************************************************************
* irrlamb - https://github.com/jazztickets/irrlamb
* Copyright (C) 2015  Alan Witkowski
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
#include <BulletDynamics/ConstraintSolver/btHingeConstraint.h>

// Constructor
_Constraint::_Constraint(const _ConstraintSpawn &Object)
:	_Object(),
	Constraint(nullptr) {

	_Template *Template = Object.Template;

	// Attributes
	if(Physics.IsEnabled()) {

		switch(Template->Type) {
			case CONSTRAINT_HINGE: {
				if(Object.BodyA) {
					Constraint = new btHingeConstraint(*Object.BodyA->GetBody(), Template->ConstraintData[0], Template->ConstraintData[1]);
					Physics.GetWorld()->addConstraint(Constraint);
				}
			} break;
		}

		if(Object.BodyA) {
/*
			if(Object.BodyB) {

				// Get local frames
				btTransform TransformA, TransformB;
				TransformA = (Object.BodyA->GetBody()->getWorldTransform().inverse() * Object.BodyB->GetBody()->getWorldTransform());
				TransformB = btTransform::getIdentity();

				// Create joint
				btGeneric6DofConstraint *NewConstraint = new btGeneric6DofConstraint(*Object.BodyA->GetBody(), *Object.BodyB->GetBody(), TransformA, TransformB, true);

				// Set limits
				NewConstraint->setLinearLowerLimit(Template->LinearLimit[0]);
				NewConstraint->setLinearUpperLimit(Template->LinearLimit[1]);
				NewConstraint->setAngularLowerLimit(Template->AngularLimit[0]);
				NewConstraint->setAngularUpperLimit(Template->AngularLimit[1]);
				Constraint = NewConstraint;
			}
			else {
				btPoint2PointConstraint *NewConstraint = new btPoint2PointConstraint(*Object.BodyA->GetBody(), btVector3(0.0f, 0.0f, 0.0f));
				Constraint = NewConstraint;
			}
*/
		}

	}

	SetProperties(Object);
}

// Destructor
_Constraint::~_Constraint() {

	if(Constraint) {
		Physics.GetWorld()->removeConstraint(Constraint);

		delete Constraint;
	}
}
