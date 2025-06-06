Feature: Combine Schedules

    Scenario: Combine two schedules and verify result
        Given we add schedules
            | name        |
            | patternA-v1 |
            | patternA-v2 |
        Then the device should have the schedule patternA-v1-v2