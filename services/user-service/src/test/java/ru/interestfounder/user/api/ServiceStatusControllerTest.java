package ru.interestfounder.user.api;

import org.junit.jupiter.api.Test;

import static org.assertj.core.api.Assertions.assertThat;

class ServiceStatusControllerTest {

    private final ServiceStatusController controller = new ServiceStatusController();

    @Test
    void reportsStubStatus() {
        var status = controller.status();

        assertThat(status.service()).isEqualTo("user-service");
        assertThat(status.status()).isEqualTo("READY");
    }
}
