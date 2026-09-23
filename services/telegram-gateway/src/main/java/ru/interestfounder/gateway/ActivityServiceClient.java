package ru.interestfounder.gateway;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.core.ParameterizedTypeReference;
import org.springframework.stereotype.Component;
import org.springframework.web.client.RestClient;

import java.time.Instant;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

@Component
public class ActivityServiceClient {

    private final RestClient client;

    public ActivityServiceClient(RestClient.Builder builder,
                                 @Value("${activity-service.base-url}") String baseUrl) {
        this.client = builder.baseUrl(baseUrl).build();
    }

    public void create(UUID userId, UUID districtId, String text) {
        client.post()
                .uri("/internal/v1/activity-sessions")
                .body(Map.of(
                        "userId", userId.toString(),
                        "districtId", districtId.toString(),
                        "text", text))
                .retrieve()
                .body(Session.class);
    }

    public Optional<Session> findActive(UUID userId) {
        var response = client.get()
                .uri("/internal/v1/activity-sessions/users/{userId}/active-session", userId)
                .retrieve()
                .toEntity(Session.class);

        return Optional.ofNullable(response.getBody());
    }

    public void cancel(UUID sessionId) {
        client.post()
                .uri("/internal/v1/activity-sessions/{sessionId}/cancel", sessionId)
                .retrieve()
                .toBodilessEntity();
    }

    public List<Session> findMatches(UUID userId) {
        return client.get()
                .uri("/internal/v1/activity-sessions/users/{userId}/matches", userId)
                .retrieve()
                .body(new ParameterizedTypeReference<>() {});
    }

    public record Session(
            UUID id,
            UUID userId,
            UUID districtId,
            String text,
            Instant startsAt,
            Instant expiresAt,
            String status
    ) {
    }
}