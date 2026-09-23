package ru.interestfounder.session.infrastructure;

import org.springframework.stereotype.Component;
import ru.interestfounder.session.domain.ActivitySession;

import java.time.Instant;
import java.util.List;
import java.util.Optional;
import java.util.UUID;

@Component
public class ActivitySessionRepository {

    private final JpaActivitySessionRepository jpa;

    public ActivitySessionRepository(JpaActivitySessionRepository jpa) {
        this.jpa = jpa;
    }

    public ActivitySession save(ActivitySession session) {
        return jpa.save(session);
    }

    public Optional<ActivitySession> findById(UUID id) {
        return jpa.findById(id);
    }

    public Optional<ActivitySession> findActiveByUserId(UUID userId) {
        var list = jpa.findActiveByUserId(userId);
        if (list.isEmpty()) {
            return Optional.empty();
        }
        return Optional.of(list.get(0));
    }

    public List<ActivitySession> findMatches(String text, UUID districtId,
                                             UUID excludeUserId, Instant now) {
        return jpa.findMatches(text, districtId, excludeUserId, now);
    }
}