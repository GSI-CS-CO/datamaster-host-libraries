Feature: Full Schedules

    Scenario: 2024-10-02 Schedule
        Given we add schedules
            | name        |
            | 2024-10-02-schedule-int |
        Then the device should have the schedule 2024-10-02-schedule-int