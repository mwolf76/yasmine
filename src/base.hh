/*
 * @file base.hh
 * @brief System wide base class definitions
 *
 * Copyright (C) 2012 Marco Pensallorto < marco AT pensallorto DOT gmail DOT com >
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 **/
#ifndef BASE_H
#define BASE_H

#include <exception>

/* the base class definition, including a virtual destructor */
class IObject {
public:
    virtual ~IObject()
    {}
};

class Object : public IObject {
};

class Exception : public std::exception {
public:
    virtual const char* what() const throw() =0;
    virtual ~Exception() throw()
    {}
};

#endif
