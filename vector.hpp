/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   vector.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akarahan <akarahan@student.42istanbul.com. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/08/15 21:21:33 by akarahan          #+#    #+#             */
/*   Updated: 2022/08/15 21:21:33 by akarahan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "./utils/type_traits.hpp"
#include "./utils/iterator.hpp"
#include "./utils/random_access_iterator.hpp"
#include "./utils/utility.hpp"

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <limits>
#include <iterator>

namespace ft
{
	template <typename T, typename Alloc = std::allocator<T> >
	class vector
	{
		public:
			typedef T													value_type;
			typedef Alloc												allocator_type;
			typedef typename allocator_type::reference					reference;
			typedef typename allocator_type::const_reference			const_reference;
			typedef typename allocator_type::pointer					pointer;
			typedef typename allocator_type::const_pointer				const_pointer;
			typedef random_access_iterator<pointer, vector>				iterator;
			typedef random_access_iterator<const_pointer, vector>		const_iterator;
			typedef typename allocator_type::size_type					size_type;
			typedef typename allocator_type::difference_type			difference_type;
			typedef ft::reverse_iterator<iterator>						reverse_iterator;
			typedef ft::reverse_iterator<const_iterator>				const_reverse_iterator;

		private:
			allocator_type _alloc;
			pointer			_start;
			pointer			_end;
			pointer			_end_cap;

		public:

			vector()
				: _alloc(allocator_type()), _start(NULL), _end(NULL), _end_cap(NULL)
			{}

			vector(const vector& other)
				: _alloc(other._alloc),
				_start(NULL),
				_end(NULL),
				_end_cap(NULL)
			{
				const size_type cap = other.capacity();
				if (cap == 0)
					return;
				_start = _alloc.allocate(cap);
				_end_cap = _start + cap;
				_end = construct_range(_start, other._start, other._end);
			}

			explicit vector( const Alloc& alloc )
				: _alloc(alloc), _start(NULL),_end(NULL), _end_cap(NULL)
			{}

			explicit vector(size_type count,
						const value_type& value = value_type(),
						const allocator_type& alloc = allocator_type())
				: _alloc(alloc), _start(NULL), _end(NULL), _end_cap(NULL)
			{
				if (count == 0)
					return ;
				check_size(count);

				_start = _alloc.allocate(count);
				_end = _start + count;
				_end_cap = _end;
				construct_range(_start, _end, value);
			}

			template< class InputIt >
			vector( InputIt first, typename enable_if<!is_integral<InputIt>::value, InputIt>::type last,
			const Alloc& alloc = Alloc() )
			: _alloc(alloc), _start(NULL), _end(NULL), _end_cap(NULL)
			{
				typedef typename iterator_traits<InputIt>::iterator_category category;
				range_init(first, last, category());
			}

			~vector()
			{
				deallocate_vector();
			}
			vector & operator=(const vector& rhs)
			{
				if (this != &rhs)
					assign(rhs.begin(), rhs.end());

				return *this;
			}

		private:
			template<typename It>
			pointer	construct_range(pointer dst, It start, It end)
			{
				for (; start != end; ++dst, (void)++start)
					_alloc.construct(dst, *start);

				return dst;
			}

			pointer construct_range(pointer dst, const_pointer end, const_reference value)
			{
				for (; dst != end; ++dst)
					_alloc.construct(dst, value);

				return dst;
			}

			template <typename InputIt>
			void range_init(InputIt first, InputIt last, std::input_iterator_tag)
			{
				for (; first != last; ++first)
					push_back(*first);
			}

			template <typename ForwardIt>
			void range_init(ForwardIt first, ForwardIt last, std::forward_iterator_tag)
			{
				const size_type count = std::distance(first, last);
				if (count == 0)
					return;
				check_size(count);

				_start = _alloc.allocate(count);
				_end_cap = _start + count;
				_end = construct_range(_start, first, last);
			}

			template <typename InputIt>
			void range_insert(iterator pos, InputIt first, InputIt last, std::input_iterator_tag)
			{
				if (pos == end())
				{
					for (; first != last; ++first)
						push_back(*first);
				}
				else if (first != last)
				{
					vector tmp(first, last);
					insert(pos, tmp.begin(), tmp.end());
				}
			}

			template <typename ForwardIt>
			void range_insert(iterator pos, ForwardIt first, ForwardIt last, std::forward_iterator_tag)
			{
				if (first != last)
				{
					const size_type count = std::distance(first, last);
					check_size(count);
					const size_type extra_space = _end_cap - _end;

					if (extra_space >= count)
					{
						const size_type elems_after = end() - pos;
						pointer old_end = _end;

						if (elems_after > count)
						{
							_end = construct_range(_end, _end - count, _end);
							std::copy_backward(pos.base(), old_end - count, old_end);
							std::copy(first, last, pos);
						}
						else
						{
							ForwardIt mid = first;
							std::advance(mid, elems_after);
							_end = construct_range(_end, mid, last);
							_end = construct_range(_end, pos.base(), old_end);
							std::copy(first, mid, pos);
						}
					}
					else
					{
						const size_type new_size = calculate_growth(count);
						pointer new_start = _alloc.allocate(new_size);
						pointer new_end = new_start;

						new_end = construct_range(new_start, _start, pos.base());
						new_end = construct_range(new_end, first, last);
						new_end = construct_range(new_end, pos.base(), _end);

						deallocate_vector();
						_start = new_start;
						_end = new_end;
						_end_cap = new_start + new_size;
					}
				}
			}

			template <typename InputIt>
			void range_assign(InputIt first, InputIt last, std::input_iterator_tag)
			{
				clear();
				for (; first != last; ++first)
					push_back(*first);

			}

			template <typename ForwardIt>
			void range_assign(ForwardIt first, ForwardIt last, std::forward_iterator_tag)
			{
				const size_type count = std::distance(first, last);

				if (count < size())
				{
					iterator it = std::copy(first, last, begin());
					erase_at_end(it.base());
				}
				else
				{
					ForwardIt it = first;
					std::advance(it, size());
					std::copy(first, it, begin());
					insert(end(), it, last);
				}
			}

			size_type calculate_growth(size_type extra) const
			{
				const size_type max = max_size();
				const size_type cap = capacity();
				if (max - cap < extra)
					length_exception();

				if (cap >= max / 2)
					return max;

				return std::max(size() + extra, cap * 2);
			}

			void destroy_range(pointer pos, pointer end)
			{
				for (; pos != end; ++pos)
					_alloc.destroy(pos);
			}

			void erase_at_end(pointer pos)
			{
				destroy_range(pos, _end);
				_end = pos;
			}

			void	length_exception() const
			{
				throw std::length_error("cannot create ft::vector larger than max_size()");
			}

			void	check_size(size_type count) const
			{
				if (count > _alloc.max_size())
					length_exception();
			}

			void	check_range(size_type pos) const
			{
				if (pos >= size())
					throw std::out_of_range("index is out of vector range");
			}

			bool	should_grow()
			{
				return _end == _end_cap;
			}

			void	deallocate_vector()
			{
				if (_start != NULL)
				{
					pointer tmp = _start;
					for (; tmp != _end; ++tmp)
						_alloc.destroy(tmp);
					_alloc.deallocate(_start, capacity());
				}
			}

		public:
			iterator begin(){ return iterator(_start);}
			const_iterator begin() const { return const_iterator(_start);}

			iterator end(){ return iterator(_end);}
			const_iterator end() const { return const_iterator(_end);}

			reverse_iterator rbegin() { return reverse_iterator(end()); }
			const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }

			reverse_iterator rend() { return reverse_iterator(begin()); }
			const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

			reference operator[]( size_type pos )
			{
				return *(_start + pos);
			}

			const_reference operator[]( size_type pos ) const
			{
				return *(_start + pos);
			}

			reference at(size_type pos)
			{
				check_range(pos);
				return (*this)[pos];
			}
			const_reference at(size_type pos) const
			{
				check_range(pos);
				return (*this)[pos];
			}

			reference front()
			{
				return *begin();
			}

			const_reference front() const
			{
				return *begin();
			}

			reference back()
			{
				return *(end() - 1);
			}

			const_reference back() const
			{
				return *(end() - 1);
			}

			pointer  data()
			{
				return _start;
			}

			const_pointer  data() const
			{
				return _start;
			}

			bool empty() const
			{
				return _start == _end;
			}
			size_type size() const
			{
				return static_cast<size_type>(std::distance(begin(), end()));
			}

			size_type max_size() const
			{
				return std::min(_alloc.max_size(),
							static_cast<size_type>(std::numeric_limits<difference_type>::max()));
			}

			void reserve( size_type new_cap )
			{
				if (capacity() >= new_cap)
					return ;
				if (new_cap > max_size())
					length_exception();

				pointer new_start = _alloc.allocate(new_cap);
				pointer new_end = construct_range(new_start, _start, _end);
				deallocate_vector();
				_start = new_start;
				_end_cap = _start + new_cap;
				_end = new_end;
			}

			size_type capacity() const
			{
				return static_cast<size_type>(std::distance(begin(), const_iterator(_end_cap)));
			}

			allocator_type get_allocator() const
			{
				return _alloc;
			}

			void assign( size_type count, const T& value )
			{
				if (count > capacity())
				{
					vector tmp(count, value);
					tmp.swap(*this);
				}
				else if (count > size())
				{
					std::fill(begin(), end(), value);
					const size_type extra = count - size();
					_end = construct_range(_end, _end + extra, value);
				}
				else
				{
					pointer it = std::fill_n(_start, count, value);
					erase_at_end(it);
				}

			}
			template <typename InputIt>
			void assign(InputIt first, typename enable_if<!is_integral<InputIt>::value, InputIt>::type last)
			{
				typedef typename iterator_traits<InputIt>::iterator_category category;
				range_assign(first, last, category());
			}

			void clear()
			{
				destroy_range(_start, _end);
				_end = _start;
			}

			iterator insert(iterator pos, const value_type& value)
			{
				const size_type index = pos - begin();

				insert(pos, 1, value);

				return iterator(_start + index);
			}

			void insert(iterator pos, size_type count, const value_type& value)
			{
				if (count != 0)
				{
					const size_type extra_space = _end_cap - _end;
					if (extra_space >= count)
					{
						const size_type elems_after = end() - pos;
						pointer old_end = _end;

						if (elems_after > count)
						{
							_end = construct_range(_end, _end - count, _end);
							std::copy_backward(pos.base(), old_end - count, old_end);
							std::fill_n(pos, count, value);
						}
						else
						{
							_end = construct_range(_end, _end + (count - elems_after), value);
							_end = construct_range(_end, pos.base(), old_end);
							std::fill(pos.base(), old_end, value);
						}
					}
					else
					{
						const size_type new_size = calculate_growth(count);
						pointer new_start = _alloc.allocate(new_size);
						pointer new_end;

						new_end = construct_range(new_start, _start, pos.base());
						new_end = construct_range(new_end, new_end + count, value);
						new_end = construct_range(new_end, pos.base(), _end);

						deallocate_vector();
						_start = new_start;
						_end = new_end;
						_end_cap = new_start + new_size;
					}
				}
			}

			template <class InputIt>
			void insert(iterator pos, InputIt first,
						typename enable_if<!is_integral<InputIt>::value, InputIt>::type last)
			{
				typedef typename iterator_traits<InputIt>::iterator_category category;
				range_insert(pos, first, last, category());
			}

			void push_back (const value_type& val)
			{
				if (!should_grow())
				{
					_alloc.construct(_end, val);
					++_end;
				}
				else
					insert(end(), val);
			}

			void	pop_back()
			{
				erase_at_end(_end - 1);
			}


			iterator erase(iterator pos)
			{
				if (pos + 1 != end())
					std::copy(pos + 1, end(), pos);
				--_end;
				_alloc.destroy(_end);
				return pos;
			}

			iterator erase(iterator first, iterator last)
			{
				const difference_type dist = std::distance(first, last);
				const difference_type index = std::distance(begin(), first);
				destroy_range(first.base(), last.base());

				if (std::distance(last, end()) > dist)
				{
					iterator it = last + dist;
					std::copy(last, it, first);
					destroy_range(last.base(), it.base());
					std::copy(it, end(), last);
					destroy_range(it.base(), end().base());
				}
				else
				{
					std::copy(last, end(), first);
					destroy_range(last.base(), end().base());
				}
				_end = _start + size() - static_cast<size_type>(dist);
				return iterator(begin() + index);
			}

			void resize (size_type count, value_type val = value_type())
			{
				const size_type len = size();
				if (count > len)
					insert(end(), count - len, val);
				else if (count < len)
					erase_at_end(_start + count);
			}

			void swap(vector& other)
			{
				std::swap(_start, other._start);
				std::swap(_end, other._end);
				std::swap(_end_cap, other._end_cap);
			}

	};

	template <typename T, typename Alloc>
	inline bool operator== (const vector<T,Alloc>& lhs, const vector<T,Alloc>& rhs)
	{
		if (lhs.size() != rhs.size())
			return false;
		return ft::equal(lhs.begin(), lhs.end(), rhs.begin());
	}

	template <typename T, typename Alloc>
	inline  bool operator!= (const vector<T,Alloc>& lhs, const vector<T,Alloc>& rhs)
	{
		return !(lhs == rhs);
	}

	template <typename T, typename Alloc>
	inline  bool operator<  (const vector<T,Alloc>& lhs, const vector<T,Alloc>& rhs)
	{
		return ft::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}

	template <typename T, typename Alloc>
	inline  bool operator>  (const vector<T,Alloc>& lhs, const vector<T,Alloc>& rhs)
	{
		return rhs < lhs;
	}

	template <typename T, typename Alloc>
	inline  bool operator<= (const vector<T,Alloc>& lhs, const vector<T,Alloc>& rhs)
	{
		return !(rhs < lhs);
	}

	template <typename T, typename Alloc>
	inline  bool operator>= (const vector<T,Alloc>& lhs, const vector<T,Alloc>& rhs)
	{
		return !(lhs < rhs);
	}
}
