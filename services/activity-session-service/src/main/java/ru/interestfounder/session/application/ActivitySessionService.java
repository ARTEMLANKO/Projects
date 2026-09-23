package ru.interestfounder.session.application;

import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import ru.interestfounder.session.domain.ActivitySession;
import ru.interestfounder.session.infrastructure.ActivitySessionRepository;

import java.time.Instant;
import java.util.List;
import java.util.Optional;
import java.util.UUID;

@Service
public class ActivitySessionService {

    private final ActivitySessionRepository repository;

    public ActivitySessionService(ActivitySessionRepository repository) {
        this.repository = repository;
    }

    @Transactional
    public ActivitySession create(UUID userId, UUID districtId, String text) {
        Instant now = Instant.now();

        repository.findActiveByUserId(userId).ifPresent(old -> {
            old.cancel();
            repository.save(old);
        });

        var session = ActivitySession.create(userId, districtId, text, now);
        return repository.save(session);
    }

    @Transactional
    public ActivitySession cancel(UUID sessionId) {
        var session = repository.findById(sessionId)
                .orElseThrow(null);
        session.cancel();
        return repository.save(session);
    }

    @Transactional(readOnly = true)
    public ActivitySession getById(UUID id) {
        return repository.findById(id)
                .orElseThrow(null);
    }

    @Transactional(readOnly = true)
    public Optional<ActivitySession> findActiveByUserId(UUID userId) {
        Instant now = Instant.now();
        return repository.findActiveByUserId(userId)
                .filter(s -> s.isEffectivelyActive(now));
    }

    @Transactional(readOnly = true)
    public List<ActivitySession> findMatches(UUID userId) {
        Instant now = Instant.now();
        var mySession = repository.findActiveByUserId(userId)
                .filter(s -> s.isEffectivelyActive(now));

        if (mySession.isEmpty()) {
            return List.of();
        }

        var s = mySession.get();
        return repository.findMatches(s.text(), s.districtId(), userId, now);
    }
}