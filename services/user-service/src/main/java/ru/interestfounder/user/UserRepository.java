package ru.interestfounder.user;

import org.springframework.data.jpa.repository.JpaRepository;

import java.util.List;
import java.util.Optional;
import java.util.UUID;

interface UserRepository extends JpaRepository<User, UUID> {
    Optional<User> findByTelegramUserId(long telegramUserId);
}

interface DistrictRepository extends JpaRepository<District, UUID> {
    List<District> findAll();
}