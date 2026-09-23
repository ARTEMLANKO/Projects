package ru.interestfounder.gateway;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.core.ParameterizedTypeReference;
import org.springframework.stereotype.Component;
import org.springframework.web.client.RestClient;

import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

@Component
public class UserServiceClient {

    private final RestClient client;

    public UserServiceClient(RestClient.Builder builder,
                             @Value("${user-service.base-url}") String baseUrl) {
        this.client = builder.baseUrl(baseUrl).build();
    }

    public Profile upsert(long telegramUserId, String username) {
        return client.put()
                .uri("/internal/v1/users/{id}", telegramUserId)
                .body(Map.of(
                        "username", username))
                .retrieve()
                .body(Profile.class);
    }

    public List<District> districts() {
        return client.get().uri("/internal/v1/districts")
                .retrieve()
                .body(new ParameterizedTypeReference<>() {}); // нужен из-за стирания типов контейнера List
    }

    public Profile selectDistrict(long telegramUserId, UUID districtId) {
        return client.put()
                .uri("/internal/v1/users/{id}/district", telegramUserId)
                .body(Map.of("districtId", districtId.toString()))
                .retrieve()
                .body(Profile.class);
    }

    public Optional<Profile> findByUUID(UUID userId) {
        var response = client.get()
                .uri("/internal/v1/users/by-id/{userId}", userId)
                .retrieve()
                .toEntity(Profile.class);
        return Optional.ofNullable(response.getBody());
    }


    public record Profile(
            UUID id,
            long telegramUserId,
            String username,
            District district
    ) {
    }

    public record District(UUID id, String name) {
    }
}