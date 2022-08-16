/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   stack.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akarahan <akarahan@student.42istanbul.com. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/08/15 21:32:21 by akarahan          #+#    #+#             */
/*   Updated: 2022/08/15 21:32:21 by akarahan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "vector.hpp"

namespace ft
{
	template <typename T, typename Container = ft::vector<T> > class stack
	{
		public:
			typedef T			value_type;
			typedef Container	container_type;
			typedef size_t		size_type;

		protected:
			container_type _c;

		public:
			explicit stack (const container_type& ctnr = container_type())
			: _c(ctnr)
			{}

			stack(const stack &rhs) : _c(rhs._c)
			{}

			stack& operator=(const stack &rhs)
			{
				if (this != &rhs)
					_c = rhs._c;
				return *this;
			}

			~stack()
			{}

		public:
			bool empty() const{
				return _c.empty();
			}

			size_type size() const{
				return _c.size();
			}

			value_type& top()
			{
				return _c.back();
			}
			const value_type& top() const{
				return _c.back();
			}

			void push (const value_type& val)
			{
				_c.push_back(val);
			}

			void pop(){
				_c.pop_back();
			}

			template <typename T1, typename C1>
			friend bool operator== (const stack<T1,C1>& lhs, const stack<T1,C1>& rhs);

    		template <typename T1, typename C1>
			friend bool operator< (const stack<T1,C1>& lhs, const stack<T1,C1>& rhs);
	};

	template <typename T1, typename C1>
	inline  bool operator== (const stack<T1,C1>& lhs, const stack<T1,C1>& rhs)
	{
		return lhs._c == rhs._c;
	}

	template <typename T1, typename C1>
	inline  bool operator!= (const stack<T1, C1>& lhs, const stack<T1, C1>& rhs)
	{
		return !(lhs == rhs);
	}

	template <typename T1, typename C1>
	inline  bool operator<  (const stack<T1, C1>& lhs, const stack<T1, C1>& rhs)
	{
		return lhs._c < rhs._c;
	}

	template <typename T1, typename C1>
	inline  bool operator<= (const stack<T1, C1>& lhs, const stack<T1, C1>& rhs)
	{
		return !(rhs < lhs);
	}

	template <typename T1, typename C1>
	inline  bool operator>  (const stack<T1, C1>& lhs, const stack<T1, C1>& rhs)
	{
		return rhs < lhs;
	}

	template <typename T1, typename C1>
	inline bool operator>= (const stack<T1, C1>& lhs, const stack<T1, C1>& rhs)
	{
		return !(lhs < rhs);
	}
}
