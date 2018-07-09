/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gmock/gmock.h>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/irange.hpp>
#include "framework/result_fixture.hpp"
#include "interfaces/iroha_internal/transaction_sequence.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

using namespace shared_model;
using ::testing::_;
using ::testing::Return;
using ::testing::Test;

class MockTransactionCollectionValidator
    : public validation::SignedTransactionsCollectionValidator<
          validation::TransactionValidator<validation::FieldValidator,
                                           validation::CommandValidatorVisitor<
                                               validation::FieldValidator>>> {
 public:
  MOCK_CONST_METHOD1(
      validatePointers,
      validation::Answer(const interface::types::SharedTxsCollectionType &));
};

/**
 * Creates transaction builder with set creator
 * @return prepared transaction builder
 */
auto prepareTransactionBuilder(std::string creator) {
  return TestTransactionBuilder().creatorAccountId(creator);
}

/**
 * Creates atomic batch from provided creator accounts
 * @param creators vector of creator account ids
 * @return atomic batch of the same size as the size of creator account ids
 */
auto createAtomicBatch(std::vector<std::string> creators) {
  std::vector<interface::types::HashType> reduced_hashes;

  for (const auto &creator : creators) {
    auto tx = prepareTransactionBuilder(creator).build();
    reduced_hashes.push_back(tx.reduced_hash());
  }

  return creators
      | boost::adaptors::transformed([reduced_hashes](const auto &creator) {
           auto res = std::shared_ptr<interface::Transaction>(
               clone(prepareTransactionBuilder(creator)
                         .batchMeta(interface::types::BatchType::ATOMIC,
                                    reduced_hashes)
                         .build()));
           return res;
         });
}

/**
 * @given Transaction collection of several transactions
 * @when create transaction sequence
 * @and transactions validator returns empty answer
 * @then TransactionSequence is created
 */
TEST(TransactionSequenceTest, CreateTransactionSequenceWhenValid) {
  MockTransactionCollectionValidator transactions_validator;

  EXPECT_CALL(transactions_validator, validatePointers(_))
      .WillOnce(Return(validation::Answer()));

  std::shared_ptr<interface::Transaction> tx(
      clone(prepareTransactionBuilder("account@domain").build()));

  auto tx_sequence = interface::TransactionSequence::createTransactionSequence(
      std::vector<decltype(tx)>{tx, tx, tx}, transactions_validator);

  ASSERT_TRUE(framework::expected::val(tx_sequence));
}

/**
 * @given Transaction collection of several transactions
 * @when create transaction sequence
 * @and transactions validator returns non empty answer
 * @then TransactionSequence is not created
 */
TEST(TransactionSequenceTest, CreateTransactionSequenceWhenInvalid) {
  MockTransactionCollectionValidator res;

  validation::Answer answer;
  answer.addReason(
      std::make_pair("transaction", std::vector<std::string>{"some reason"}));

  EXPECT_CALL(res, validatePointers(_)).WillOnce(Return(answer));

  std::shared_ptr<interface::Transaction> tx(
      clone(prepareTransactionBuilder("account@domain").build()));

  auto tx_sequence = interface::TransactionSequence::createTransactionSequence(
      std::vector<decltype(tx)>{tx, tx, tx}, res);

  ASSERT_FALSE(framework::expected::val(tx_sequence));
}

/**
 * @given Transaction collection of several transactions, including some of the
 * united into the batches
 * @when transactions validator returns empty answer
 * @and create transaction sequence
 * @then expected number of batches is created
 */
TEST(TransactionSequenceTest, CreateBatches) {
  MockTransactionCollectionValidator txs_validator;

  EXPECT_CALL(txs_validator, validatePointers(_))
      .WillOnce(Return(validation::Answer()));

  size_t batches_number = 3;
  size_t txs_in_batch = 2;
  size_t single_transactions = 1;

  interface::types::SharedTxsCollectionType tx_collection;
  for (size_t i = 0; i < batches_number; i++) {
    std::vector<std::string> creators;
    for (size_t j = 0; j < txs_in_batch; j++) {
      creators.push_back("batch" + std::to_string(i) + "account"
                         + std::to_string(j) + "@domain");
    }

    auto batch = createAtomicBatch(creators);
    tx_collection.insert(tx_collection.begin(), batch.begin(), batch.end());
  }
  for (size_t i = 0; i < single_transactions; i++) {
    tx_collection.emplace_back(
        clone(prepareTransactionBuilder("single_tx_account@domain"
                                        + std::to_string(i))
                  .build()));
  }

  auto tx_sequence_opt =
      interface::TransactionSequence::createTransactionSequence(tx_collection,
                                                                txs_validator);

  auto tx_sequence = framework::expected::val(tx_sequence_opt);
  ASSERT_TRUE(tx_sequence);

  ASSERT_EQ(boost::size(tx_sequence->value.transactions()),
            batches_number * txs_in_batch + single_transactions);
  ASSERT_EQ(boost::size(tx_sequence->value.batches()),
            batches_number + single_transactions);
}
