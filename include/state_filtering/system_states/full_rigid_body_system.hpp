/*************************************************************************
This software allows for filtering in high-dimensional measurement and
state spaces, as described in

M. Wuthrich, P. Pastor, M. Kalakrishnan, J. Bohg, and S. Schaal.
Probabilistic Object Tracking using a Range Camera
IEEE/RSJ Intl Conf on Intelligent Robots and Systems, 2013

In a publication based on this software pleace cite the above reference.


Copyright (C) 2014  Manuel Wuthrich

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*************************************************************************/


#ifndef FULL_RIGID_BODY_SYSTEM_HPP_
#define FULL_RIGID_BODY_SYSTEM_HPP_

#include <state_filtering/system_states/rigid_body_system.hpp>

#include <Eigen/Dense>
#include <boost/static_assert.hpp>


#include <vector>




template<int body_size>
struct FullRigidBodySystemTypes
{
    enum
    {
        COUNT_PER_BODY = 13,
        POSITION_INDEX = 0,
        POSITION_COUNT = 3,
        ORIENTATION_INDEX = 3,
        ORIENTATION_COUNT = 4,
        LINEAR_VELOCITY_INDEX = 7,
        LINEAR_VELOCITY_COUNT = 3,
        ANGULAR_VELOCITY_INDEX = 10,
        ANGULAR_VELOCITY_COUNT = 3
    };

    typedef RigidBodySystem<body_size == -1 ? -1 : body_size * COUNT_PER_BODY> Base;
};

template<int body_size = -1>
class FullRigidBodySystem: public FullRigidBodySystemTypes<body_size>::Base
{
public:
    typedef FullRigidBodySystemTypes<body_size> Types;
    typedef typename Types::Base Base;
    typedef typename Base::StateVector StateVector;
    typedef typename Base::Quaternion Quaternion;
    typedef typename Base::RotationMatrix RotationMatrix;
    typedef typename Base::HomogeneousMatrix HomogeneousMatrix;
    typedef typename Base::Position Position;
    typedef Eigen::Vector4d Orientation;
    typedef typename Base::LinearVelocity LinearVelocity;
    typedef typename Base::AngularVelocity AngularVelocity;

    enum
    {
        SIZE_BODIES = body_size,
        SIZE_STATE = Base::SIZE_STATE,
        COUNT_PER_BODY = Types::COUNT_PER_BODY,
        POSITION_INDEX = Types::POSITION_INDEX,
        POSITION_COUNT = Types::POSITION_COUNT,
        ORIENTATION_INDEX = Types::ORIENTATION_INDEX,
        ORIENTATION_COUNT = Types::ORIENTATION_COUNT,
        LINEAR_VELOCITY_INDEX = Types::LINEAR_VELOCITY_INDEX,
        LINEAR_VELOCITY_COUNT = Types::LINEAR_VELOCITY_COUNT,
        ANGULAR_VELOCITY_INDEX = Types::ANGULAR_VELOCITY_INDEX,
        ANGULAR_VELOCITY_COUNT = Types::ANGULAR_VELOCITY_COUNT
    };

    typedef Eigen::VectorBlock<StateVector, POSITION_COUNT>         PositionBlock;
    typedef Eigen::VectorBlock<StateVector, ORIENTATION_COUNT>      OrientationBlock;
    typedef Eigen::VectorBlock<StateVector, LINEAR_VELOCITY_COUNT>  LinearVelocityBlock;
    typedef Eigen::VectorBlock<StateVector, ANGULAR_VELOCITY_COUNT> AngularVelocityBlock;
    typedef Eigen::VectorBlock<StateVector, COUNT_PER_BODY>         SingleBodyBlock;




    // constructor for static size without initial value -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    FullRigidBodySystem():
        Base(StateVector::Zero(SIZE_STATE),
             body_size)
    {
        assert_fixed_size<true>();
        reset_quaternions();
    }
    // constructor for dynamic size without initial value -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    FullRigidBodySystem(unsigned count_bodies):
        Base(StateVector::Zero(count_bodies * COUNT_PER_BODY),
             count_bodies)
    {
        assert_dynamic_size<true>();
        reset_quaternions();
    }
    // constructor with initial value -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    template <typename T> FullRigidBodySystem(const Eigen::MatrixBase<T>& state_vector):
        Base(state_vector,
             state_vector.rows()/COUNT_PER_BODY){ }

    virtual ~FullRigidBodySystem() {}

    // implementation of get fcts from parent class --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    virtual Position get_position(const size_t& object_index = 0) const
    {
        return this->template middleRows<POSITION_COUNT>(object_index * COUNT_PER_BODY + POSITION_INDEX);
    }
    virtual Orientation get_orientation(const size_t& object_index = 0) const
    {
        return this->template middleRows<ORIENTATION_COUNT>(object_index * COUNT_PER_BODY + ORIENTATION_INDEX);
    }
    virtual Quaternion get_quaternion(const size_t& object_index = 0) const
    {
        return Quaternion(get_orientation(object_index));
    }
    virtual LinearVelocity get_linear_velocity(const size_t& object_index = 0) const
    {
        return this->template middleRows<LINEAR_VELOCITY_COUNT>(object_index * COUNT_PER_BODY + LINEAR_VELOCITY_INDEX);
    }
    virtual AngularVelocity get_angular_velocity(const size_t& object_index = 0) const
    {
        return this->template middleRows<ANGULAR_VELOCITY_COUNT>(object_index * COUNT_PER_BODY + ANGULAR_VELOCITY_INDEX);
    }

    // child specific fcts ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    PositionBlock position(const size_t& object_index = 0)
    {
      return PositionBlock(this->derived(), object_index * COUNT_PER_BODY + POSITION_INDEX);
    }
    OrientationBlock orientation(const size_t& object_index = 0)
    {
      return OrientationBlock(this->derived(), object_index * COUNT_PER_BODY + ORIENTATION_INDEX);
    }
    LinearVelocityBlock linear_velocity(const size_t& object_index = 0)
    {
      return LinearVelocityBlock(this->derived(), object_index * COUNT_PER_BODY + LINEAR_VELOCITY_INDEX);
    }
    AngularVelocityBlock angular_velocity(const size_t& object_index = 0)
    {
      return AngularVelocityBlock(this->derived(), object_index * COUNT_PER_BODY + ANGULAR_VELOCITY_INDEX);
    }

    SingleBodyBlock operator [](const size_t& object_index)
    {
      return SingleBodyBlock(this->derived(), object_index * COUNT_PER_BODY);
    }



    // counts
    virtual unsigned countState() const
    {
        return Base::countState();
    }
    virtual unsigned countBodies() const
    {
        return Base::countBodies();
    }
private:
    template <bool dummy> void assert_fixed_size() const {BOOST_STATIC_ASSERT(body_size > -1);}
    template <bool dummy> void assert_dynamic_size() const {BOOST_STATIC_ASSERT(body_size == -1);}

    void reset_quaternions()
    {
        for(size_t object_index = 0; object_index < countBodies(); object_index++)
            this->template middleRows<4>(object_index * COUNT_PER_BODY +  3) = Quaternion::Identity().coeffs();
    }
};



#endif