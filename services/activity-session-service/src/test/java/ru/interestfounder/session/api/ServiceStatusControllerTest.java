package ru.interestfounder.session.api;

import org.junit.jupiter.api.Test;

import static org.assertj.core.api.Assertions.assertThat;

class ServiceStatusControllerTest {

    private final ServiceStatusController controller = new ServiceStatusController();

    @Test
    void reportsStubStatus() {
        var status = controller.status();

        assertThat(status.service()).isEqualTo("activity-session-service");
        assertThat(status.status()).isEqualTo("STUB");
    }
}
