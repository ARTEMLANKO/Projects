package ru.interestfounder.session.api;

import org.springframework.web.bind.annotation.*;
import ru.interestfounder.session.application.ActivitySessionService;
import ru.interestfounder.session.domain.ActivitySession;
import ru.interestfounder.session.domain.ActivitySessionStatus;

import java.time.Instant;
import java.util.List;
import java.util.UUID;

@RestController
@RequestMapping("/internal/v1/activity-sessions")
public class ActivitySessionController {

    private final ActivitySessionService service;

    public ActivitySessionController(ActivitySessionService service) {
        this.service = service;
    }

    @PostMapping
    public SessionResponse create(@RequestBody CreateSessionRequest request) {
        ActivitySession session = service.create(
                request.userId(),
                request.districtId(),
                request.text()
        );
        return SessionResponse.from(session);
    }

    @GetMapping("/{sessionId}")
    public SessionResponse get(@PathVariable UUID sessionId) {
        return SessionResponse.from(service.getById(sessionId));
    }

    @PostMapping("/{sessionId}/cancel")
    public SessionResponse cancel(@PathVariable UUID sessionId) {
        return SessionResponse.from(service.cancel(sessionId));
    }

    @GetMapping("/users/{userId}/active-session")
    public SessionResponse getActive(@PathVariable UUID userId) {
        return service.findActiveByUserId(userId)
                .map(SessionResponse::from)
                .orElse(null);
    }

    @GetMapping("/users/{userId}/matches")
    public List<SessionResponse> getMatches(@PathVariable UUID userId) {
        return service.findMatches(userId).stream()
                .map(SessionResponse::from)
                .toList();
    }
}

record CreateSessionRequest(
        UUID userId,
        UUID districtId,
        String text
) {
}

record SessionResponse(
        UUID id,
        UUID userId,
        UUID districtId,
        String text,
        Instant startsAt,
        Instant expiresAt,
        ActivitySessionStatus status
) {
    public static SessionResponse from(ActivitySession s) {
        return new SessionResponse(
                s.id(),
                s.userId(),
                s.districtId(),
                s.text(),
                s.startsAt(),
                s.expiresAt(),
                s.status()
        );
    }
}