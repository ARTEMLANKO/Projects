package ru.interestfounder.user;

import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.webmvc.test.autoconfigure.AutoConfigureMockMvc;
import org.springframework.http.MediaType;
import org.springframework.test.context.DynamicPropertyRegistry;
import org.springframework.test.context.DynamicPropertySource;
import org.springframework.test.web.servlet.MockMvc;
import org.testcontainers.junit.jupiter.Container;
import org.testcontainers.junit.jupiter.Testcontainers;
import org.testcontainers.postgresql.PostgreSQLContainer;
import org.testcontainers.utility.DockerImageName;
import ru.interestfounder.user.application.UserProfileService;

import static org.assertj.core.api.Assertions.assertThat;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.put;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

@Testcontainers
@SpringBootTest
@AutoConfigureMockMvc
class UserServiceIntegrationTest {

    @Container
    static final PostgreSQLContainer POSTGRES =
            new PostgreSQLContainer(DockerImageName.parse("postgres:17-alpine"));

    @DynamicPropertySource
    static void postgresProperties(DynamicPropertyRegistry registry) {
        registry.add("spring.datasource.url", POSTGRES::getJdbcUrl);
        registry.add("spring.datasource.username", POSTGRES::getUsername);
        registry.add("spring.datasource.password", POSTGRES::getPassword);
        registry.add("spring.data.redis.repositories.enabled", () -> false);
    }

    @Autowired
    private UserProfileService userProfileService;

    @Autowired
    private MockMvc mockMvc;

    @Test
    void migrationsSeedAllSaintPetersburgDistrictsInStableOrder() {
        var districts = userProfileService.getActiveDistricts();

        assertThat(districts).hasSize(18);
        assertThat(districts.getFirst().getCode()).isEqualTo("ADMIRALTEYSKY");
        assertThat(districts.getLast().getCode()).isEqualTo("TSENTRALNY");
    }

    @Test
    void createsUpdatesAndAssignsDistrictToTelegramUser() {
        long telegramUserId = 123_456_789L;

        var created = userProfileService.upsert(
                telegramUserId,
                "interest_user",
                "Александр",
                null
        );
        var district = userProfileService.getActiveDistricts().get(14);
        userProfileService.selectDistrict(telegramUserId, district.getId());
        var updated = userProfileService.upsert(
                telegramUserId,
                "interest_user",
                "Александр",
                "Алаев"
        );
        var loaded = userProfileService.getByTelegramUserId(telegramUserId);

        assertThat(updated.getId()).isEqualTo(created.getId());
        assertThat(loaded.getProfile().getLastName()).isEqualTo("Алаев");
        assertThat(loaded.getProfile().getDistrict().getCode()).isEqualTo("PRIMORSKY");
    }

    @Test
    void exposesUserOnboardingThroughRestApi() throws Exception {
        long telegramUserId = 987_654_321L;

        mockMvc.perform(get("/internal/v1/districts"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.length()").value(18))
                .andExpect(jsonPath("$[0].code").value("ADMIRALTEYSKY"));

        mockMvc.perform(put("/internal/v1/users/{telegramUserId}", telegramUserId)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "username": "rest_user",
                                  "firstName": "Артём",
                                  "lastName": "Ланко"
                                }
                                """))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.telegramUserId").value(telegramUserId))
                .andExpect(jsonPath("$.firstName").value("Артём"));

        mockMvc.perform(put("/internal/v1/users/{telegramUserId}/district", telegramUserId)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "districtId": "00000000-0000-0000-0000-000000000015"
                                }
                                """))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.district.code").value("PRIMORSKY"));

        mockMvc.perform(get("/internal/v1/users/{telegramUserId}", telegramUserId))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.username").value("rest_user"))
                .andExpect(jsonPath("$.district.name").value("Приморский район"));
    }
}
