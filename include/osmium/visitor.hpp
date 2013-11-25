#ifndef OSMIUM_VISITOR_HPP
#define OSMIUM_VISITOR_HPP

/*

This file is part of Osmium (http://osmcode.org/osmium).

Copyright 2013 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <stdexcept>
#include <type_traits>

#include <osmium/osm.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/io/input_iterator.hpp>

namespace osmium {

    namespace handler {
        class Handler;
    }

    namespace visitor {

        namespace detail {

            template <typename T, typename U>
            using MaybeConst = typename std::conditional<std::is_const<T>::value, typename std::add_const<U>::type, U>::type;

            template <class TVisitor, typename TItem>
            inline void switch_on_type(TVisitor& visitor, TItem& item, std::false_type) {
                switch (item.type()) {
                    case osmium::item_type::node:
                        visitor(static_cast<MaybeConst<TItem, osmium::Node>&>(item));
                        break;
                    case osmium::item_type::way:
                        visitor(static_cast<MaybeConst<TItem, osmium::Way>&>(item));
                        break;
                    case osmium::item_type::relation:
                        visitor(static_cast<MaybeConst<TItem, osmium::Relation>&>(item));
                        break;
                    case osmium::item_type::changeset:
                        visitor(static_cast<MaybeConst<TItem, osmium::Changeset>&>(item));
                        break;
                    case osmium::item_type::tag_list:
                        visitor(static_cast<MaybeConst<TItem, osmium::TagList>&>(item));
                        break;
                    case osmium::item_type::way_node_list:
                        visitor(static_cast<MaybeConst<TItem, osmium::WayNodeList>&>(item));
                        break;
                    case osmium::item_type::relation_member_list:
                    case osmium::item_type::relation_member_list_with_full_members:
                        visitor(static_cast<MaybeConst<TItem, osmium::RelationMemberList>&>(item));
                        break;
                    default:
                        throw std::runtime_error("unknown type");
                }
            }

            template <class TVisitor, class TItem>
            inline void switch_on_type(TVisitor& visitor, TItem& item, std::true_type) {
                switch (item.type()) {
                    case osmium::item_type::node:
                        visitor.node(static_cast<MaybeConst<TItem, osmium::Node>&>(item));
                        break;
                    case osmium::item_type::way:
                        visitor.way(static_cast<MaybeConst<TItem, osmium::Way>&>(item));
                        break;
                    case osmium::item_type::relation:
                        visitor.relation(static_cast<MaybeConst<TItem, osmium::Relation>&>(item));
                        break;
                    case osmium::item_type::changeset:
                        visitor.changeset(static_cast<MaybeConst<TItem, osmium::Changeset>&>(item));
                        break;
                    default:
                        throw std::runtime_error("unknown type");
                }
            }

            template <class TVisitor>
            inline void switch_on_type(TVisitor& visitor, osmium::Object& item, std::true_type) {
                switch (item.type()) {
                    case osmium::item_type::node:
                        visitor.node(static_cast<osmium::Node&>(item));
                        break;
                    case osmium::item_type::way:
                        visitor.way(static_cast<osmium::Way&>(item));
                        break;
                    case osmium::item_type::relation:
                        visitor.relation(static_cast<osmium::Relation&>(item));
                        break;
                    default:
                        throw std::runtime_error("unknown type");
                }
            }

            template <class TVisitor>
            inline void switch_on_type(TVisitor& visitor, const osmium::Object& item, std::true_type) {
                switch (item.type()) {
                    case osmium::item_type::node:
                        visitor.node(static_cast<const osmium::Node&>(item));
                        break;
                    case osmium::item_type::way:
                        visitor.way(static_cast<const osmium::Way&>(item));
                        break;
                    case osmium::item_type::relation:
                        visitor.relation(static_cast<const osmium::Relation&>(item));
                        break;
                    default:
                        throw std::runtime_error("unknown type");
                }
            }

            template <class TVisitor, class TItem>
            inline void apply_item_recurse(TItem& item, TVisitor& visitor) {
                typename std::is_base_of<osmium::handler::Handler, TVisitor>::type tag;
                switch_on_type(visitor, item, tag);
            }

            template <class TVisitor, class TItem, class ...TRest>
            inline void apply_item_recurse(TItem& item, TVisitor& visitor, TRest&... more) {
                apply_item_recurse(item, visitor);
                apply_item_recurse(item, more...);
            }

            template <class TVisitor>
            inline void switch_on_type_before_after(osmium::item_type last, osmium::item_type current, TVisitor& visitor, std::false_type) {
                // inentionally left blank
            }

            template <class TVisitor>
            inline void switch_on_type_before_after(osmium::item_type last, osmium::item_type current, TVisitor& visitor, std::true_type) {
                switch (last) {
                    case osmium::item_type::undefined:
                        visitor.init();
                        break;
                    case osmium::item_type::node:
                        visitor.after_nodes();
                        break;
                    case osmium::item_type::way:
                        visitor.after_ways();
                        break;
                    case osmium::item_type::relation:
                        visitor.after_relations();
                        break;
                    case osmium::item_type::changeset:
                        visitor.after_changesets();
                        break;
                    default:
                        break;
                }
                switch (current) {
                    case osmium::item_type::undefined:
                        visitor.done();
                        break;
                    case osmium::item_type::node:
                        visitor.before_nodes();
                        break;
                    case osmium::item_type::way:
                        visitor.before_ways();
                        break;
                    case osmium::item_type::relation:
                        visitor.before_relations();
                        break;
                    case osmium::item_type::changeset:
                        visitor.before_changesets();
                        break;
                    default:
                        break;
                }
            }

            template <class TVisitor>
            inline void apply_before_and_after_recurse(osmium::item_type last, osmium::item_type current, TVisitor& visitor) {
                typename std::is_base_of<osmium::handler::Handler, TVisitor>::type tag;
                switch_on_type_before_after(last, current, visitor, tag);
            }

            template <class TVisitor, class ...TRest>
            inline void apply_before_and_after_recurse(osmium::item_type last, osmium::item_type current, TVisitor& visitor, TRest&... more) {
                apply_before_and_after_recurse(last, current, visitor);
                apply_before_and_after_recurse(last, current, more...);
            }

        } // namespace detail

    } // namespace visitor

    template <class ...TVisitors>
    inline void apply_item(const osmium::memory::Item& item, TVisitors&... visitors) {
        osmium::visitor::detail::apply_item_recurse(item, visitors...);
    }

    template <class ...TVisitors>
    inline void apply_item(osmium::memory::Item& item, TVisitors&... visitors) {
        osmium::visitor::detail::apply_item_recurse(item, visitors...);
    }

    template <class TIterator, class ...TVisitors>
    inline void apply(TIterator it, TIterator end, TVisitors&... visitors) {
        osmium::item_type last_type = osmium::item_type::undefined;
        for (; it != end; ++it) {
            if (last_type != it->type()) {
                osmium::visitor::detail::apply_before_and_after_recurse(last_type, it->type(), visitors...);
                last_type = it->type();
            }
            osmium::visitor::detail::apply_item_recurse(*it, visitors...);
        }
        osmium::visitor::detail::apply_before_and_after_recurse(last_type, osmium::item_type::undefined, visitors...);
    }

    template <class TSource, class ...TVisitors>
    inline void apply(TSource& source, TVisitors&... visitors) {
        apply(osmium::io::InputIterator<TSource>{source},
              osmium::io::InputIterator<TSource>{},
              visitors...);
    }

    template <class ...TVisitors>
    inline void apply(osmium::memory::Buffer& buffer, TVisitors&... visitors) {
        apply(buffer.begin(), buffer.end(), visitors...);
    }

    template <class ...TVisitors>
    inline void apply(const osmium::memory::Buffer& buffer, TVisitors&... visitors) {
        apply(buffer.cbegin(), buffer.cend(), visitors...);
    }

} // namespace osmium

#endif // OSMIUM_VISITOR_HPP
