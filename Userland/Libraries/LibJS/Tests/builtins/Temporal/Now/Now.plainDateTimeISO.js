describe("correct behavior", () => {
    test("length is 0", () => {
        expect(Temporal.Now.plainDateTimeISO).toHaveLength(0);
    });

    test("basic functionality", () => {
        const plainDateTime = Temporal.Now.plainDateTimeISO();
        expect(plainDateTime).toBeInstanceOf(Temporal.PlainDateTime);
        expect(plainDateTime.calendar.id).toBe("iso8601");
    });

    test("custom time zone", () => {
        const timeZone = {
            getOffsetNanosecondsFor() {
                return 86400000000000;
            },
        };

        const plainDateTimeToEpochSeconds = plainDateTime =>
            (plainDateTime.year - 1970) * 31_556_952 +
            plainDateTime.month * 2_630_000 +
            plainDateTime.day * 86_400 +
            plainDateTime.hour * 3_600 +
            plainDateTime.minute * 60 +
            plainDateTime.second +
            plainDateTime.millisecond / 1_000 +
            plainDateTime.microsecond / 1_000_000 +
            plainDateTime.nanosecond / 1_000_000_000;

        const plainDateTime = Temporal.Now.plainDateTimeISO();
        const plainDateTimeWithOffset = Temporal.Now.plainDateTimeISO(timeZone);
        // Let's hope the duration between the above two lines is less than a second :^)
        const differenceSeconds =
            plainDateTimeToEpochSeconds(plainDateTimeWithOffset) -
            plainDateTimeToEpochSeconds(plainDateTime);
        expect(Math.floor(differenceSeconds)).toBe(86400);
    });
});
