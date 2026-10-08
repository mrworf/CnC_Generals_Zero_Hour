/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: INIException.h ///////////////////////////////////////////////////////////////////////////
// Author: John McDonald, Jr, October 2002
// Desc:   INI Exception class. Thrown when INIs fail to read.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <exception>
#include <string>
#include <utility>
class INIException : public std::exception
{
	// This is a stack based exception class. It is used to output useful information
	// when thrown from an INI message

private:
	std::string m_message;
	bool m_present=false;
	void bind() noexcept {mFailureMessage=m_present?m_message.data():nullptr;}
public:
	char* mFailureMessage=nullptr; // retained original diagnostic API, owned here
	explicit INIException(const char* message):m_message(message?message:""),m_present(message!=nullptr){bind();}
	INIException(const INIException& other):m_message(other.m_message),m_present(other.m_present){bind();}
	INIException(INIException&& other) noexcept:m_message(std::move(other.m_message)),m_present(other.m_present){bind();other.m_present=false;other.bind();}
	INIException& operator=(INIException other) noexcept {
		m_message.swap(other.m_message);std::swap(m_present,other.m_present);bind();other.bind();return *this;
	}
	const char* what() const noexcept override{return m_message.c_str();}
};
