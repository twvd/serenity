/*
 * Copyright (c) 2021, Idan Horowitz <idan.horowitz@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibJS/Runtime/AbstractOperations.h>
#include <LibJS/Runtime/GlobalObject.h>
#include <LibJS/Runtime/Temporal/AbstractOperations.h>
#include <LibJS/Runtime/Temporal/Calendar.h>
#include <LibJS/Runtime/Temporal/CalendarConstructor.h>
#include <LibJS/Runtime/Temporal/PlainDate.h>
#include <LibJS/Runtime/Value.h>

namespace JS::Temporal {

// 12 Temporal.Calendar Objects, https://tc39.es/proposal-temporal/#sec-temporal-calendar-objects
Calendar::Calendar(String identifier, Object& prototype)
    : Object(prototype)
    , m_identifier(move(identifier))
{
}

// 12.1.1 CreateTemporalCalendar ( identifier [ , newTarget ] ), https://tc39.es/proposal-temporal/#sec-temporal-createtemporalcalendar
Calendar* create_temporal_calendar(GlobalObject& global_object, String const& identifier, FunctionObject* new_target)
{
    auto& vm = global_object.vm();

    // 1. Assert: ! IsBuiltinCalendar(identifier) is true.
    VERIFY(is_builtin_calendar(identifier));

    // 2. If newTarget is not provided, set newTarget to %Temporal.Calendar%.
    if (!new_target)
        new_target = global_object.temporal_calendar_constructor();

    // 3. Let object be ? OrdinaryCreateFromConstructor(newTarget, "%Temporal.Calendar.prototype%", « [[InitializedTemporalCalendar]], [[Identifier]] »).
    // 4. Set object.[[Identifier]] to identifier.
    auto* object = ordinary_create_from_constructor<Calendar>(global_object, *new_target, &GlobalObject::temporal_calendar_prototype, identifier);
    if (vm.exception())
        return {};

    // 5. Return object.
    return object;
}

// 12.1.2 IsBuiltinCalendar ( id ), https://tc39.es/proposal-temporal/#sec-temporal-isbuiltincalendar
// NOTE: This is the minimum IsBuiltinCalendar implementation for engines without ECMA-402.
bool is_builtin_calendar(String const& identifier)
{
    // 1. If id is not "iso8601", return false.
    if (identifier != "iso8601"sv)
        return false;

    // 2. Return true.
    return true;
}

// 12.1.3 GetBuiltinCalendar ( id )
Calendar* get_builtin_calendar(GlobalObject& global_object, String const& identifier)
{
    auto& vm = global_object.vm();

    // 1. If ! IsBuiltinCalendar(id) is false, throw a RangeError exception.
    if (!is_builtin_calendar(identifier)) {
        vm.throw_exception<RangeError>(global_object, ErrorType::TemporalInvalidCalendarIdentifier, identifier);
        return {};
    }

    // 2. Return ? Construct(%Temporal.Calendar%, « id »).
    MarkedValueList arguments(vm.heap());
    arguments.append(js_string(vm, identifier));
    auto calendar = vm.construct(*global_object.temporal_calendar_constructor(), *global_object.temporal_calendar_constructor(), move(arguments));
    if (vm.exception())
        return {};
    return static_cast<Calendar*>(&calendar.as_object());
}

// 12.1.4 GetISO8601Calendar ( )
Calendar* get_iso8601_calendar(GlobalObject& global_object)
{
    // 1. Return ? GetBuiltinCalendar("iso8601").
    return get_builtin_calendar(global_object, "iso8601");
}

// 12.1.21 ToTemporalCalendar ( temporalCalendarLike ), https://tc39.es/proposal-temporal/#sec-temporal-totemporalcalendar
Object* to_temporal_calendar(GlobalObject& global_object, Value temporal_calendar_like)
{
    auto& vm = global_object.vm();

    // 1. If Type(temporalCalendarLike) is Object, then
    if (temporal_calendar_like.is_object()) {
        auto& temporal_calendar_like_object = temporal_calendar_like.as_object();
        // a. If temporalCalendarLike has an [[InitializedTemporalDate]], [[InitializedTemporalDateTime]], [[InitializedTemporalMonthDay]], [[InitializedTemporalTime]], [[InitializedTemporalYearMonth]], or [[InitializedTemporalZonedDateTime]] internal slot, then
        // TODO: The rest of the Temporal built-ins
        if (is<PlainDate>(temporal_calendar_like_object)) {
            // i. Return temporalCalendarLike.[[Calendar]].
            return &static_cast<PlainDate&>(temporal_calendar_like_object).calendar();
        }

        // b. If ? HasProperty(temporalCalendarLike, "calendar") is false, return temporalCalendarLike.
        auto has_property = temporal_calendar_like_object.has_property(vm.names.calendar);
        if (vm.exception())
            return {};
        if (!has_property)
            return &temporal_calendar_like_object;

        // c. Set temporalCalendarLike to ? Get(temporalCalendarLike, "calendar").
        temporal_calendar_like = temporal_calendar_like_object.get(vm.names.calendar);
        if (vm.exception())
            return {};
        // d. If Type(temporalCalendarLike) is Object and ? HasProperty(temporalCalendarLike, "calendar") is false, return temporalCalendarLike.
        if (temporal_calendar_like.is_object()) {
            has_property = temporal_calendar_like.as_object().has_property(vm.names.calendar);
            if (vm.exception())
                return {};
            if (!has_property)
                return &temporal_calendar_like.as_object();
        }
    }

    // 2. Let identifier be ? ToString(temporalCalendarLike).
    auto identifier = temporal_calendar_like.to_string(global_object);
    if (vm.exception())
        return {};

    // 3. If ! IsBuiltinCalendar(identifier) is false, then
    if (!is_builtin_calendar(identifier)) {
        // a. Let identifier be ? ParseTemporalCalendarString(identifier).
        auto parsed_identifier = parse_temporal_calendar_string(global_object, identifier);
        if (vm.exception())
            return {};
        identifier = move(*parsed_identifier);
    }

    // 4. Return ? CreateTemporalCalendar(identifier).
    return create_temporal_calendar(global_object, identifier);
}

// 12.1.22 ToTemporalCalendarWithISODefault ( temporalCalendarLike ), https://tc39.es/proposal-temporal/#sec-temporal-totemporalcalendarwithisodefault
Object* to_temporal_calendar_with_iso_default(GlobalObject& global_object, Value temporal_calendar_like)
{
    // 1. If temporalCalendarLike is undefined, then
    if (temporal_calendar_like.is_undefined()) {
        // a. Return ? GetISO8601Calendar().
        return get_iso8601_calendar(global_object);
    }
    // 2. Return ? ToTemporalCalendar(temporalCalendarLike).
    return to_temporal_calendar(global_object, temporal_calendar_like);
}

// 12.1.30 IsISOLeapYear ( year ), https://tc39.es/proposal-temporal/#sec-temporal-isisoleapyear
bool is_iso_leap_year(i32 year)
{
    // 1. Assert: year is an integer.

    // 2. If year modulo 4 ≠ 0, return false.
    if (year % 4 != 0)
        return false;

    // 3. If year modulo 400 = 0, return true.
    if (year % 400 == 0)
        return true;

    // 4. If year modulo 100 = 0, return false.
    if (year % 100 == 0)
        return false;

    // 5. Return true.
    return true;
}

// 12.1.32 ISODaysInMonth ( year, month ), https://tc39.es/proposal-temporal/#sec-temporal-isodaysinmonth
i32 iso_days_in_month(i32 year, i32 month)
{
    // 1. Assert: year is an integer.

    // 2. Assert: month is an integer, month ≥ 1, and month ≤ 12.
    VERIFY(month >= 1 && month <= 12);

    // 3. If month is 1, 3, 5, 7, 8, 10, or 12, return 31.
    if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12)
        return 31;

    // 4. If month is 4, 6, 9, or 11, return 30.
    if (month == 4 || month == 6 || month == 9 || month == 11)
        return 30;

    // 5. If ! IsISOLeapYear(year) is true, return 29.
    if (is_iso_leap_year(year))
        return 29;

    // 6. Return 28.
    return 28;
}

// 12.1.36 BuildISOMonthCode ( month ), https://tc39.es/proposal-temporal/#sec-buildisomonthcode
String build_iso_month_code(i32 month)
{
    return String::formatted("M{:02}", month);
}

// 12.1.37 ResolveISOMonth ( fields ), https://tc39.es/proposal-temporal/#sec-temporal-resolveisomonth
double resolve_iso_month(GlobalObject& global_object, Object& fields)
{
    auto& vm = global_object.vm();

    // 1. Let month be ? Get(fields, "month").
    auto month = fields.get(vm.names.month);
    if (vm.exception())
        return {};

    // 2. Let monthCode be ? Get(fields, "monthCode").
    auto month_code = fields.get(vm.names.monthCode);
    if (vm.exception())
        return {};

    // 3. If monthCode is undefined, then
    if (month_code.is_undefined()) {
        // a. If month is undefined, throw a TypeError exception.
        if (month.is_undefined()) {
            vm.throw_exception<TypeError>(global_object, ErrorType::TemporalMissingRequiredProperty, vm.names.month.as_string());
            return {};
        }
        // b. Return month.
        return month.as_double();
    }

    // 4. Assert: Type(monthCode) is String.
    VERIFY(month_code.is_string());
    auto& month_code_string = month_code.as_string().string();
    // 5. Let monthLength be the length of monthCode.
    auto month_length = month_code_string.length();
    // 6. If monthLength is not 3, throw a RangeError exception.
    if (month_length != 3) {
        vm.throw_exception<RangeError>(global_object, ErrorType::TemporalInvalidMonthCode);
        return {};
    }
    // 7. Let numberPart be the substring of monthCode from 1.
    auto number_part = month_code_string.substring(1);
    // 8. Set numberPart to ! ToIntegerOrInfinity(numberPart).
    auto number_part_integer = Value(js_string(vm, move(number_part))).to_integer_or_infinity(global_object);
    // 9. If numberPart < 1 or numberPart > 12, throw a RangeError exception.
    if (number_part_integer < 1 || number_part_integer > 12) {
        vm.throw_exception<RangeError>(global_object, ErrorType::TemporalInvalidMonthCode);
        return {};
    }
    // 10. If month is not undefined, and month ≠ numberPart, then
    if (!month.is_undefined() && month.as_double() != number_part_integer) {
        // a. Throw a RangeError exception.
        vm.throw_exception<RangeError>(global_object, ErrorType::TemporalInvalidMonthCode);
        return {};
    }
    // 11. If ! SameValueNonNumeric(monthCode, ! BuildISOMonthCode(numberPart)) is false, then
    if (month_code_string != build_iso_month_code(number_part_integer)) {
        // a. Throw a RangeError exception.
        vm.throw_exception<RangeError>(global_object, ErrorType::TemporalInvalidMonthCode);
        return {};
    }
    // 12. Return numberPart.
    return number_part_integer;
}

// 12.1.38 ISODateFromFields ( fields, options ), https://tc39.es/proposal-temporal/#sec-temporal-isodatefromfields
Optional<TemporalDate> iso_date_from_fields(GlobalObject& global_object, Object& fields, Object& options)
{
    auto& vm = global_object.vm();

    // 1. Assert: Type(fields) is Object.

    // 2. Let overflow be ? ToTemporalOverflow(options).
    auto overflow = to_temporal_overflow(global_object, options);
    if (vm.exception())
        return {};

    // 3. Set fields to ? PrepareTemporalFields(fields, « "day", "month", "monthCode", "year" », «»).
    auto* prepared_fields = prepare_temporal_fields(global_object, fields, { "day", "month", "monthCode", "year" }, {});
    if (vm.exception())
        return {};

    // 4. Let year be ? Get(fields, "year").
    auto year = prepared_fields->get(vm.names.year);
    if (vm.exception())
        return {};

    // 5. If year is undefined, throw a TypeError exception.
    if (year.is_undefined()) {
        vm.throw_exception<TypeError>(global_object, ErrorType::TemporalMissingRequiredProperty, vm.names.year.as_string());
        return {};
    }

    // 6. Let month be ? ResolveISOMonth(fields).
    auto month = resolve_iso_month(global_object, *prepared_fields);
    if (vm.exception())
        return {};

    // 7. Let day be ? Get(fields, "day").
    auto day = prepared_fields->get(vm.names.day);
    if (vm.exception())
        return {};

    // 8. If day is undefined, throw a TypeError exception.
    if (day.is_undefined()) {
        vm.throw_exception<TypeError>(global_object, ErrorType::TemporalMissingRequiredProperty, vm.names.day.as_string());
        return {};
    }

    // 9. Return ? RegulateISODate(year, month, day, overflow).
    return regulate_iso_date(global_object, year.as_double(), month, day.as_double(), *overflow);
}

}
